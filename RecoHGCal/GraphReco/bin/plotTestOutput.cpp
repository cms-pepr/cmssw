/*
 * plotOutput.C
 *
 *  Created on: 27 Sep 2019
 *      Author: jkiesele
 */
#include "TFile.h"
#include "TTree.h"
#include "TH2D.h"
#include "TCanvas.h"
#include <iostream>


TCanvas * projectRechitsEtaPhi(TTree* t,
        const std::vector<float>  * rh_energy,
        const std::vector<float>  * rh_eta,
        const std::vector<float>  * rh_phi,
        const std::vector<float>  * sc_eta,
        const std::vector<float>  * sc_phi, int event){

    TCanvas * cv = new TCanvas();
    float maxenergy = 0;
    for(const auto& e:*rh_energy){
        if(e>maxenergy)
            maxenergy=e;
    }

    std::vector<int> colors = {kBlue,kCyan,kGreen,kOrange,kRed};
    std::vector<float> sizes = {0.1,  0.3,  0.5,  0.7,     0.9};
    std::vector<float> delim = {0.0, 0.02,  0.05,  0.1,  0.3,     1.0};

    for(size_t i=0;i<colors.size();i++){

        t->SetMarkerStyle(8);
        t->SetMarkerColor(colors.at(i));
        t->SetMarkerSize(sizes.at(i));

        TString cutstr="recHitEnergy >";
        cutstr+=delim.at(i)*maxenergy;
        cutstr+= " && recHitEnergy <";
        cutstr+=delim.at(i+1)*maxenergy;
        if(i)
            t->Draw("recHitEta:recHitRelPhi",cutstr,"same",1,event);
        else
            t->Draw("recHitEta:recHitRelPhi",cutstr,"",1,event);


        //continue;

    }
    t->SetMarkerStyle(5);
    t->SetMarkerSize(1);
    t->SetMarkerColor(kBlack);
    t->Draw("truthSimclusterEtas:truthSimclusterPhis","","same",1,event);


    return cv;
}






int main(int argc, char* argv[]){

    if(argc<2)
        return -1;
    TString infile = argv[1];

    int maxevents=10;
    if(argc>2)
        maxevents=atoi(argv[2]);

    TFile f(infile,"READ");
    TTree * tree = (TTree *)f.Get("WindowNTupler/tree");
    if(!tree || tree->IsZombie())
        return -1;
    int nentries = tree->GetEntries();

    std::cout << "nentries " << nentries << std::endl;


    std::vector<float>  * recHitEnergy=0, * recHitEta=0, * recHitRelPhi=0, * truthSimclusterEtas=0, * truthSimclusterPhis=0;


    tree->SetBranchAddress("recHitEnergy", &recHitEnergy);
    tree->SetBranchAddress("recHitEta", &recHitEta);
    tree->SetBranchAddress("recHitRelPhi", &recHitRelPhi);
    tree->SetBranchAddress("truthSimclusterEtas",&truthSimclusterEtas);
    tree->SetBranchAddress("truthSimclusterPhis",&truthSimclusterPhis);

    TFile fout("outfile.root","RECREATE");
    for(int event=0; event < nentries;event++){
        if(event>maxevents)
            break;
        tree->GetEntry(event);

        auto * cv = projectRechitsEtaPhi(tree,recHitEnergy,recHitEta,recHitRelPhi,truthSimclusterEtas,truthSimclusterPhis,event);

        TString outname="projectedEtaPhi";
        outname+=event;
        cv->SetTitle(outname);
        cv->SetName(outname);
        outname+=".pdf";
        cv->Print(outname);
        cv->Write();
        delete cv;
    }
    fout.Close();

return 0;

}


