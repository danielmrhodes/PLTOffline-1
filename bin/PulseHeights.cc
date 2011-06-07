////////////////////////////////////////////////////////////////////
//
// Dean Andrew Hidas <Dean.Andrew.Hidas@cern.ch>
//
// Created on: Thu May 26 16:47:32 CEST 2011
//
////////////////////////////////////////////////////////////////////


#include <iostream>
#include <string>
#include <map>

#include "PLTEvent.h"

#include "TH2F.h"
#include "TCanvas.h"
#include "TLegend.h"




// FUNCTION DEFINITIONS HERE
int PulseHeights (std::string const, std::string const);






// CODE BELOW




int PulseHeights (std::string const DataFileName, std::string const GainCalFileName)
{

  int const HistColors[4] = { 1, 4, 28, 2 };


  // Grab the plt event reader
  PLTEvent Event(DataFileName, GainCalFileName);

  // Map for all ROC hists and canvas
  std::map<int, std::vector<TH1F*> > hMap;
  std::map<int, TCanvas*> cMap;

  // Loop over all events in file
  for (int ientry = 0; Event.GetNextEvent() >= 0; ++ientry) {
    for (size_t iTelescope = 0; iTelescope != Event.NTelescopes(); ++iTelescope) {
      PLTTelescope* Telescope = Event.Telescope(iTelescope);
      for (size_t iPlane = 0; iPlane != Telescope->NPlanes(); ++iPlane) {
        PLTPlane* Plane = Telescope->Plane(iPlane);

        if (Plane->ROC() > 2) {
          std::cerr << "WARNING: ROC > 2 found: " << Plane->ROC() << std::endl;
          continue;
        }
        if (Plane->Channel() > 99) {
          std::cerr << "WARNING: Channel > 99 found: " << Plane->Channel() << std::endl;
          continue;
        }

        // ID the plane and roc by 3 digit number
        int const id = 10*Plane->Channel() + Plane->ROC();

        if (!hMap.count(id)) {
          hMap[id].push_back( new TH1F( TString::Format("Pulse Height for Ch %02i ROC %1i Pixels All", (int) Plane->Channel(), (int) Plane->ROC()),
                TString::Format("PulseHeight_Ch%02i_ROC%1i_All", (int) Plane->Channel(), (int) Plane->ROC()), 100, 0, 65000) );
          for (size_t ih = 1; ih != 4; ++ih) {
            hMap[id].push_back( new TH1F( TString::Format("Pulse Height for Ch %02i ROC %1i Pixels %i", (int) Plane->Channel(), (int) Plane->ROC(), (int) ih),
                   TString::Format("PulseHeight_Ch%02i_ROC%1i_Pixels%i", (int) Plane->Channel(), (int) Plane->ROC(), (int) ih), 100, 0, 65000) );
          }
          // If we're making a new hist I'd say there's a 1 in 3 chance we'll need a canvas for it
          if (!cMap.count(Plane->Channel())) {
            // Create canvas with given name
            TString BUFF;
            BUFF.Form("PulseHeight_Ch%02i",  (int) Plane->Channel());
            std::cout << "Creating New Canvas: " << BUFF << std::endl;
            cMap[Plane->Channel()] = new TCanvas(BUFF, BUFF, 900, 300);
            cMap[Plane->Channel()]->Divide(3,1);
          }


        }


        // Loop over all clusters on this plane
        for (size_t iCluster = 0; iCluster != Plane->NClusters(); ++iCluster) {
          PLTCluster* Cluster = Plane->Cluster(iCluster);

          size_t NHits = Cluster->NHits();

          hMap[id][0]->Fill( Cluster->Charge() );
          if (NHits == 1) {
            hMap[id][1]->Fill( Cluster->Charge() );
          } else if (NHits == 2) {
            hMap[id][2]->Fill( Cluster->Charge() );
          } else if (NHits == 3) {
            hMap[id][3]->Fill( Cluster->Charge() );
          }
        }



      }
    }
  }


  // Loop over all histograms and draw them on the correct canvas in the correct pad
  for (std::map<int, std::vector<TH1F*> >::iterator it = hMap.begin(); it != hMap.end(); ++it) {

    // Decode the ID
    int const Channel = it->first / 10;
    int const ROC     = it->first % 10;

    printf("Drawing hists for Channel %2i ROC %i\n", Channel, ROC);

    // change to correct pad on canvas and draw the hist
    cMap[Channel]->cd(ROC+1);

    for (size_t ih = 0; ih != 4; ++ih) {
      TH1F* Hist = it->second[ih];

      Hist->SetLineColor(HistColors[ih]);
      if (ih == 0) {
        Hist->SetTitle( TString::Format("PulseHeight Ch%02i ROC%1i", Channel, ROC) );
        Hist->SetXTitle("Electrons");
        Hist->SetYTitle("Events");
        Hist->Draw("hist");
      } else {
        Hist->Draw("samehist");
      }
    }

  }

  // Loop over all canvas, save them, and delete them
  for (std::map<int, TCanvas*>::iterator it = cMap.begin(); it != cMap.end(); ++it) {
    it->second->SaveAs( it->second->GetName()+TString(".gif") );
    delete it->second;
  }

  return 0;
}


int main (int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [DataFileName] [GainCalFileName]" << std::endl;
    return 1;
  }

  std::string const DataFileName = argv[1];
  std::string const GainCalFileName = argv[2];
  std::cout << "DataFileName:    " << DataFileName << std::endl;
  std::cout << "GainCalFileName: " << GainCalFileName << std::endl;

  PulseHeights(DataFileName, GainCalFileName);

  return 0;
}