# coding: utf-8

"""
Plots for evaluating the realistic HGCal truth accumulation.
"""

import os
import math
from array import array

import six

import plotlib.root as r
import ROOT


colors = {
    0: ROOT.kWhite,
    22: ROOT.kRed,
    11: ROOT.kOrange,
    13: ROOT.kBlue,
}


def pdgid_to_color(pdg_id):
    return colors.get(abs(pdg_id), colors[0])


def p4_in_hgcal(p4):
    return 1.52 <= abs(p4.eta()) < 3.00


def fwlite_loop(path, handle_data=None, start=0, end=-1, object_type="Event"):
    """
    Opens one or more ROOT files defined by *path* and yields the FWLite event. When *handle_data*
    is not *None*, it is supposed to be a dictionary ``key -> {"type": ..., "label": ...}``. In that
    case, the handle products are yielded as well in a dictionary, mapped to the key, as
    ``(event, objects dict)``.
    """
    import ROOT
    ROOT.PyConfig.IgnoreCommandLineOptions = True
    ROOT.gROOT.SetBatch()
    ROOT.gSystem.Load("libFWCoreFWLite.so")
    ROOT.gSystem.Load("libDataFormatsFWLite.so")
    ROOT.FWLiteEnabler.enable()
    from DataFormats.FWLite import Events, Runs, Handle  # noqa: F401

    paths = path if isinstance(path, (list, tuple)) else [path]

    handles = {}
    if handle_data:
        for key, data in handle_data.items():
            handles[key] = Handle(data["type"])

    objects = locals()[object_type + "s"](paths)
    for i, obj in enumerate(objects):
        if i < start:
            continue
        if i >= end:
            break

        if handle_data:
            products = {}
            for key, data in handle_data.items():
                obj.getByLabel(data["label"], handles[key])
                products[key] = handles[key].product()
            yield obj, products
        else:
            yield obj


def plot_radii(input_file, output_file=""):
    if not output_file:
        postfix = os.path.splitext(os.path.basename(input_file))[0].split("_", 1)[-1]
        output_file = "radii_{}.png".format(postfix)

    r.setup_style()
    canvas, (pad,) = r.routines.create_canvas()
    pad.cd()

    binning = (51, -0.02, 2.02)
    hist = ROOT.TH1F("hist", ";SimCluster radius (#eta - #phi);# Normalized entries", *binning)

    # fill data
    handles = {
        "radii": {"type": "vector<float>", "label": "mix:RealisticCaloTruth:HLT"},
    }
    for event, objects in fwlite_loop(input_file, handles, end=1):
        radii = objects["radii"]
        for radius in radii:
            hist.Fill(radius)

    n_cluster = int(hist.Integral())
    hist.Scale(1. / n_cluster)

    r.setup_hist(hist, pad=pad)
    r.tools.set_color(hist, 2, "lm")

    hist.Draw("hist")

    label = r.routines.create_top_right_label("{} SimClusters".format(n_cluster),
        h_offset=0.04, v_offset=0.07)
    label.Draw()

    r.update_canvas(canvas)
    canvas.SaveAs(output_file)


def plot_clusters(input_file, output_file="", description="", event=0):
    if not output_file:
        postfix = os.path.splitext(os.path.basename(input_file))[0].split("_", 1)[-1]
        output_file = "clusters_{}_evt{}.png".format(postfix, event)

    r.setup_style()
    canvas, (pad,) = r.routines.create_canvas()
    pad.cd()

    # dummy histogram for axes
    binning = (1, -5., 5., 1, -3.2, 5.)
    dummy_hist = ROOT.TH2F("hist", ";#eta;#phi;", *binning)

    # coordinates of calo particles
    cp_points = []

    # fill data
    handles = {
        "radii": {
            "type": "vector<float>",
            "label": "mix:RealisticCaloTruth:DIGI",
        },
        "sim_clusters": {
            "type": "vector<SimCluster>",
            "label": "mix:MergedCaloTruth:DIGI",
        },
        "calo_particles": {
            "type": "vector<CaloParticle>",
            "label": "mix:MergedCaloTruth:DIGI",
        },
        "shower_vectors": {
            "type": "vector<math::XYZTLorentzVectorD>",
            "label": "mix:RealisticCaloTruth:DIGI",
        },
        "merged_clusters": {
            "type": "vector<SimCluster>",
            "label": "mix:RealisticCaloTruth:DIGI",
        },
    }
    ellipses = []
    for event, objects in fwlite_loop(input_file, handles, start=event, end=event + 1):
        radii = objects["radii"]
        sim_clusters = objects["sim_clusters"]
        calo_particles = objects["calo_particles"]
        shower_vectors = objects["shower_vectors"]
        merged_clusters = objects["merged_clusters"]

        det_ids = set()
        n_sim_clusters = 0
        for radius, sim_cluster, shower_vector in six.moves.zip(radii, sim_clusters, shower_vectors):
            if not p4_in_hgcal(sim_cluster.p4()):
                continue

            det_ids |= set(hf.first for hf in sim_cluster.hits_and_fractions())
            n_sim_clusters += 1

            # if abs(p4.eta()) + radius < 5 and abs(p4.phi()) + radius < 3.2:
            # if len(sim_cluster.hits_and_fractions()) >= 200:
            # if sim_cluster.p4().E() > 0.8:
            if 1:
                print sim_cluster.p4().E(), shower_vector.E(), shower_vector.Eta(), shower_vector.Phi()
                # radius = 0.1 * (math.log(sim_cluster.p4().E()) + 1)
                ellipse = ROOT.TEllipse(shower_vector.Eta(), shower_vector.Phi(), radius, radius)
                r.setup_ellipse(ellipse, {"FillColor": pdgid_to_color(sim_cluster.pdgId())})
                ellipses.append(ellipse)

        for calo_particle in calo_particles:
            cp_points.append((calo_particle.eta(), calo_particle.phi()))

        # counts for some statistics
        n_rec_hits = len(det_ids)
        n_calo_particles = sum(1 for cp in calo_particles if p4_in_hgcal(cp.p4()))
        n_merged_clusters = len(merged_clusters)

    r.setup_hist(dummy_hist, pad=pad)
    dummy_hist.Draw()

    for ellipse in sorted(ellipses, key=lambda e: -e.GetR1()):
        ellipse.Draw()

    cp_graph = ROOT.TGraph(len(cp_points), array("f", [p[0] for p in cp_points]),
        array("f", [p[1] for p in cp_points]))
    r.setup_graph(cp_graph, {"MarkerStyle": 43, "MarkerSize": 2})
    cp_graph.Draw("P")

    # stats labels
    def create_stat_label(i, j, text):
        v = 0.07 + i * 0.04
        h = 0.4 + j * 0.23
        return r.routines.create_top_left_label(text, h_offset=h, v_offset=v)

    create_stat_label(0, 0, "{} RecHits".format(n_rec_hits)).Draw()
    create_stat_label(0, 1, "{} CaloParticles".format(n_calo_particles)).Draw()
    create_stat_label(1, 0, "{} SimClusters".format(n_sim_clusters)).Draw()
    create_stat_label(1, 1, "{} Merged SCs".format(n_merged_clusters)).Draw()

    # geometry and energy label
    r.routines.create_top_right_label("14 TeV (2026D41)", v_offset=-0.005).Draw()

    # cms label
    for l in r.routines.create_cms_labels(postfix="Simulation"):
        l.Draw()

    # optional description
    if description:
        if not isinstance(description, list):
            description = [description]
        for i, desc in enumerate(description):
            r.routines.create_top_left_label(desc, h_offset=0.03, v_offset=0.07 + i * 0.04).Draw()

    # box hiding the phi labels larger than 3
    box = ROOT.TBox(-5.5, 3.5, -5.05, 5.2)
    r.setup_box(box, color="white")
    box.Draw()

    r.update_canvas(canvas)
    canvas.SaveAs(output_file)


if __name__ == "__main__":
    # plot_radii("digi_gun_e_e50To50_z0To0_n1_pu200.root")
    # plot_clusters("digi_gun_e_e50To50_z0To0_n1_pu200.root",
    #     description="Electron gun, 50 GeV, 200 PU")
    for i in range(5):
        plot_clusters("digi_gun_e_e30To30_z0To0_n5_pu0.root",
            description="Electron gun, 30 GeV, 0 PU", event=i)
    # for i in range(3):
    #     plot_clusters("digi_gun_g_e30To30_z0To0_n5_pu0.root",
    #         description="Photon gun, 30 GeV, 0 PU", event=i)
