// Methods in this file:
// BPrime_reco_new(), BPrime_rec(), BPrime_reco_alt()

// ----------------------------------------------------------------------------------------------------------------------------------------------------------------
// Fxn to return any and all float TPrime and BPrime variables needed for plotting
// ----------------------------------------------------------------------------------------------------------------------------------------------------------------

using namespace std;
using namespace ROOT::VecOps;

RVec<float> BPrime_reco_new(TLorentzVector top_lv, TLorentzVector W_lv, string sample, int min_M_lep_Jet, int min_M_lep_JetID, int NOSJets_central, int NOSJets_DeepFlavM, int NSSJets_central, int NSSJets_DeepFlavM, RVec<int> gcSSJet_DeepFlavM, RVec<int> gcOSJet_DeepFlavM, RVec<double> gcOSFatJet_pt, RVec<double> gcOSFatJet_eta, RVec<double> gcOSFatJet_phi, RVec<double> gcOSFatJet_mass, RVec<double> gcOSFatJet_tag, RVec<float> gcOSJet_pt, RVec<float> gcOSJet_eta, RVec<float> gcOSJet_phi, RVec<float> gcOSJet_mass, RVec<float> gcSSJet_pt, RVec<float> gcSSJet_eta, RVec<float> gcSSJet_phi, RVec<float> gcSSJet_mass)
{
  // Initialize variables for the two canidates.  These are added together towards the end of the method
  
  TLorentzVector Tcand_lv, Wcand_lv;

  // Flags to know which case was found and if a case was found at all
  // taggedTjet = 1, taggedWjet = 2, untaggedTlep = 3, untaggedWlep = 4
  float Bdecay_obs = 0;

  // Set up b tagged jet booleans
  bool SSbJet = false, OSbJet = false, SSmljJet = false;
  TLorentzVector BjetOS_lv, BjetSS_lv, W_jet_lv, fatJet;
  int value = 0, i = 0;

  // Find out whether the highest pt other side AK8 (fat jet) is t-tagged
  // If so, find where it is and make it into a TLorentzVector
  int HighestPt = ArgMax(gcOSFatJet_pt);
  bool ttag = false, wtag = false;
  if (gcOSFatJet_tag[HighestPt] == 1) {
    ttag = true;
  } else if (gcOSFatJet_tag[HighestPt] == 2) {
    wtag = true;
  }
  fatJet.SetPtEtaPhiM(gcOSFatJet_pt[HighestPt], gcOSFatJet_eta[HighestPt], gcOSFatJet_phi[HighestPt], gcOSFatJet_mass[HighestPt]);
  

  // --- SS BJets Method --- If true, use the BTagging method, if false, use the minMlj method
  bool BTag = true;

  // BTagging Method: Now we check whether there are any same side b-tagged jets
  if (NSSJets_DeepFlavM > 0 && BTag == true) {
    SSbJet = true;
    for (i = 0; value != 0; i++) {
      value = gcSSJet_DeepFlavM[i];
    }
    BjetSS_lv.SetPtEtaPhiM(gcSSJet_pt[i], gcSSJet_eta[i], gcSSJet_phi[i], gcSSJet_mass[i]);
  }

  // minMlj Method: Save whether minMlj is less than 173
  if (min_M_lep_Jet < 173 && BTag == false) {
    SSmljJet = true;
  }

  // Now we check whether there are any other side b-tagged jets
  // If so, find where it is and make it into a TLorentzVector
  if (NOSJets_DeepFlavM > 0) {
    OSbJet = true;
    value = 0;
    for (i = 0; value != 0; i++) {
      value = gcOSJet_DeepFlavM[i];
      if (value != 0) {
        BjetOS_lv.SetPtEtaPhiM(gcOSJet_pt[i], gcOSJet_eta[i], gcOSJet_phi[i], gcOSJet_mass[i]);
        double dr = fatJet.DeltaR(BjetOS_lv);
        if (dr < 0.8) {
          value = 0;
        }
      }
    }
  }


  // --- 5 Cases --- This is where we split into the four different cases and make our canidates
  if (ttag && !SSbJet) 
  {
    // Checking Case 1: requires highest-pt other side t-tagged AK8 (fat jet), and requires no same side b-tagged AK4 (jet)
    Wcand_lv = W_lv;
    Tcand_lv = fatJet;
    Bdecay_obs = 1;
  }
  else if (wtag && SSbJet)
  {
    // Checking Case 2: requires highest-pt other side w-tagged AK8 (fat jet), requires a good same side b-tagged jet or min_M_lep_Jet < 173
    Wcand_lv = fatJet;
    if (BTag == true) {
      Tcand_lv = W_lv + BjetSS_lv; // Bjet should be set above in the B jet if statement
    } else { //BTag == false
      Tcand_lv = top_lv;
    }
    Bdecay_obs = 2;
  }
  
  // else if (wtag && OSbJet)
  // {
  //   //Checking Case 3: requires highest-pt other side w-tagged AK8 (fat jet), requires other side b-tagged jet to make a top quark
  //   Wcand_lv = W_lv;
  //   Tcand_lv = fatJet + BjetOS_lv;
  //   Bdecay_obs = 3;
  // }
  else
  {
    //The event doesn't match any of the three cases
    //First check if there is a b-tagged same side jet
    //Question: do I set validBDecay equal to something in here?
    if (SSbJet) {
      Wcand_lv = fatJet;
      Tcand_lv = W_lv + BjetSS_lv;
      Bdecay_obs = 3;
    } else {
      if (!OSbJet) {
        Tcand_lv = fatJet;
      } else {
        Tcand_lv = fatJet + BjetOS_lv;
      }
      Wcand_lv = W_lv;
      Bdecay_obs = 4;
    }
  }

  // Setting the value of Bdecay_true depending on inputs to this method if it is a BPrime sample
  // W_gen_pt > 200: 1, W_gen_pt <= 200: 2, t_gen_pt > 400: 3, t_gen_pt <= 400: 4

  // Using the canidates from above, make a BPrime and store various attributes of it
  TLorentzVector Bprime_lv = Wcand_lv + Tcand_lv;
  float Bprime_mass = Bprime_lv.M();
  float Bprime_pt = Bprime_lv.Pt();
  float Bprime_eta = Bprime_lv.Eta();
  float Bprime_phi = Bprime_lv.Phi();
  float Bprime_DR = Wcand_lv.DeltaR(Tcand_lv);
  float Bprime_ptbal = Wcand_lv.Pt() / Tcand_lv.Pt();
  float Bprime_chi2 = pow(Tcand_lv.M() - 172.6, 2) / (19.1 * 19.1) + pow(Wcand_lv.M() - 85.5, 2) / (8.7 * 8.7) + pow(Bprime_DR - TMath::Pi(), 2) / (0.2 * 0.2) + pow(Bprime_ptbal - 1, 2) / (0.8 * 0.8); // leptonic t, hadronic W chi2 with values from 2016 B2G-17-018

  // // Make a vector with the stored values and return it
  RVec<float> BPrimeVec = {Bprime_mass, Bprime_pt, Bprime_eta, Bprime_phi, Bprime_DR, Bprime_ptbal, Bprime_chi2, Bdecay_obs};
  return BPrimeVec;
}











RVec<float> BPrime_reco(TLorentzVector top_lv, TLorentzVector Wlv, int leptonicParticle, RVec<float> &ak8_pt, RVec<float> &ak8_eta, RVec<float> &ak8_phi, RVec<float> &ak8_mass, RVec<int> &dpak8_tag, RVec<float> &ak8_sdmass)
{
  RVec<int> validBTagged(2, 0);
  int val = -99;
  int tag = -99;
  TLorentzVector jet_lv;
  vector<pair<TLorentzVector, int>> jets_lv;
  float DR_LP_AK8 = 0;
  bool isLeptonic_W = false;
  bool isLeptonic_t = false;

  for (unsigned int ijet = 0; ijet < ak8_pt.size(); ijet++)
  {
    jet_lv.SetPtEtaPhiM(ak8_pt.at(ijet), ak8_eta.at(ijet), ak8_phi.at(ijet), ak8_mass.at(ijet));
    if (leptonicParticle == 0)
    {
      DR_LP_AK8 = jet_lv.DeltaR(Wlv);
      isLeptonic_W = true;
    }
    if (leptonicParticle == 1)
    {
      DR_LP_AK8 = jet_lv.DeltaR(top_lv);
      isLeptonic_t = true;
    }
    if (jets_lv.size() > 1)
      continue; // we only take 2 for single Bprime

    if (jet_lv.DeltaR(top_lv) > 0.8 and isLeptonic_t)
    {
      jets_lv.push_back(make_pair(jet_lv, ijet));
    }
    if (jet_lv.DeltaR(Wlv) > 0.8 and isLeptonic_W)
    {
      jets_lv.push_back(make_pair(jet_lv, ijet));
    }
  }

  float Bprime_mass = -999;
  float Bprime_pt = -999;
  float Bprime_eta = 9;
  float Bprime_phi = 9;
  float Bprime_DR = -9;
  float Bprime_ptbal = -999;
  float Bprime_chi2 = -999;

  TLorentzVector Wcand_lv;
  TLorentzVector Tcand_lv;
  TLorentzVector Bprime_lv;
  float validBDecay = -1;

  float taggedTjet = 0;
  float taggedTjet2nd = 0;
  float taggedWbjetJet = 0;
  float taggedWjet = 0;
  float taggedWjet2nd = 0;
  bool is2Jet = false;

  // ----------------------------------------------------------------------------
  // VLQ Decay -- 1 or 2 AK8 jets away from leptonic particle
  // ----------------------------------------------------------------------------
  if (jets_lv.size() > 2)
  {
    cout << "Problem: > 2 AK8s for Bprime reco" << endl;
  }
  else if (jets_lv.size() == 0)
  {
    cout << "Failed reco event, no jets" << endl;
  }

  int npass_reco = 0;

  if (jets_lv.size() == 1 || jets_lv.size() == 2)
  {
    npass_reco = npass_reco + 1;

    if (jets_lv.size() == 2)
    {
      is2Jet = true;
    }

    // get the tags
    float jet1_tag = dpak8_tag.at(jets_lv.at(0).second);
    float jet2_tag = -1;
    if (is2Jet)
      jet2_tag = dpak8_tag.at(jets_lv.at(1).second);

    // pair up the jet tag with the pT index 0,1,2 and sort by tag (orders J, T, W,)
    vector<pair<float, float>> decayJets;
    decayJets.push_back(make_pair(jet1_tag, 0));
    if (is2Jet)
      decayJets.push_back(make_pair(jet2_tag, 1));
    sort(decayJets.begin(), decayJets.end());

    // Start forming 2 particle groups
    validBDecay = -1;

    // Derived from Previous logic: TT -> tZ bW, BB -> tW bZ
    if (isLeptonic_W)
    {

      Wcand_lv = Wlv; // W candidate is always the leptonic W here

      if (decayJets.at(0).first == 1)
      { // leading jet is top tagged
        validBDecay = 1;
        taggedTjet = 1;
        Tcand_lv = jets_lv.at(decayJets.at(0).second).first; // decayJets.second gives the jets_lv index to get 4-vec
      }
      else if (is2Jet && decayJets.at(1).first == 1)
      {                  // 2nd-leading jet is top tagged
        validBDecay = 0; // 0 for suboptimal validity
        taggedTjet = 1;
        Tcand_lv = jets_lv.at(decayJets.at(1).second).first;
      }
      else
      { // leading jets are not top tagged
        if (!is2Jet)
        {
          validBDecay = -1;                                    // all other 1-jet events are really mistagged
          Tcand_lv = jets_lv.at(decayJets.at(0).second).first; // take the jet as the T candidate with a bad tag
        }
        else
        { // it's a 2-jet event, will assume partially-merged top and add the two jets, looking for a W tag
          if (decayJets.at(0).first == 2)
          { // W tag found
            validBDecay = 1;
            taggedWbjetJet = 1;
            Tcand_lv = jets_lv.at(decayJets.at(0).second).first + jets_lv.at(decayJets.at(1).second).first;
          }
          else if (decayJets.at(1).first == 2)
          {                  // W tag found
            validBDecay = 0; // sub-optimal
            taggedWbjetJet = 1;
            Tcand_lv = jets_lv.at(decayJets.at(0).second).first + jets_lv.at(decayJets.at(1).second).first;
          }
          else
          { // any other set of tags
            validBDecay = -1;
            Tcand_lv = jets_lv.at(decayJets.at(0).second).first + jets_lv.at(decayJets.at(1).second).first;
          }
        }
      }

      Bprime_DR = Wcand_lv.DeltaR(Tcand_lv);
      Bprime_ptbal = Wcand_lv.Pt() / Tcand_lv.Pt();
      Bprime_chi2 = pow(Tcand_lv.M() - 170.1, 2) / (14.29 * 14.29) + pow(Bprime_DR - TMath::Pi(), 2) / (0.2 * 0.2) + pow(Bprime_ptbal - 1, 2) / (0.8 * 0.8);
      // hadronic t chi2 with values from 2016 B2G-17-018 (W isn't there because W_lv uses mass constraint)
    }
    else
    { // isLeptonic_t

      Tcand_lv = top_lv; // T candidate is always the leptonic top here

      if (decayJets.at(0).first == 2)
      { // leading jet is W-tagged
        validBDecay = 1;
        taggedWjet = 1;
        Wcand_lv = jets_lv.at(decayJets.at(0).second).first;
      }
      else if (is2Jet && decayJets.at(1).first == 2)
      {                  // 2nd-leading jet is W-tagged
        validBDecay = 0; // sub-optimal
        taggedWjet = 1;
        Wcand_lv = jets_lv.at(decayJets.at(1).second).first;
      }
      else
      {
        validBDecay = -1;
        Wcand_lv = jets_lv.at(decayJets.at(0).second).first; // let's assume a 1-jet mistag scenario
      }

      Bprime_DR = Wcand_lv.DeltaR(Tcand_lv);
      Bprime_ptbal = Wcand_lv.Pt() / Tcand_lv.Pt();
      Bprime_chi2 = pow(Tcand_lv.M() - 172.6, 2) / (19.1 * 19.1) + pow(Wcand_lv.M() - 85.5, 2) / (8.7 * 8.7) + pow(Bprime_DR - TMath::Pi(), 2) / (0.2 * 0.2) + pow(Bprime_ptbal - 1, 2) / (0.8 * 0.8); // leptonic t, hadronic W chi2 with values from 2016 B2G-17-018
    }

    Bprime_lv = Wcand_lv + Tcand_lv;
    Bprime_mass = Bprime_lv.M();
    Bprime_pt = Bprime_lv.Pt();
    Bprime_eta = Bprime_lv.Eta();
    Bprime_phi = Bprime_lv.Phi();
  }

  // Include validBDecy_DeepAK8 variable and what it's decayed as
  // leptonic and hadronic jetIDs can be slimmed out, no longer exist
  RVec<float> BPrimeVec = {Bprime_mass, Bprime_pt, Bprime_eta, Bprime_phi, Bprime_DR, Bprime_ptbal, Bprime_chi2, validBDecay, taggedWbjetJet, taggedTjet, taggedWjet};
  return BPrimeVec;
};
















///////////////////////
//    Alternative    //
///////////////////////
RVec<float> BPrime_reco_alt(TLorentzVector lepton_lv, TLorentzVector top_lv, TLorentzVector Wlv, int leptonicParticle, RVec<float> &ak8_pt, RVec<float> &ak8_eta, RVec<float> &ak8_phi, RVec<float> &ak8_mass, RVec<int> &dpak8_tag, RVec<float> &ak8_sdmass)
{
  RVec<int> validBTagged(2, 0);
  int val = -99;
  int tag = -99;
  TLorentzVector jet_lv;
  vector<pair<TLorentzVector, int>> jets_lv;
  float DR_LP_AK8 = 0;
  bool isLeptonic_W = false;
  bool isLeptonic_t = false;
  RVec<float> DR_lep(ak8_pt.size(), 9);
  int n_ak8 = ak8_pt.size();

  for (unsigned int ijet = 0; ijet < n_ak8; ijet++)
  {
    jet_lv.SetPtEtaPhiM(ak8_pt.at(ijet), ak8_eta.at(ijet), ak8_phi.at(ijet), ak8_mass.at(ijet));
    DR_lep[ijet] = abs(jet_lv.DeltaR(lepton_lv) - 3);
  }
  DR_lep = Argsort(DR_lep);

  for (unsigned int i = 0; i < n_ak8; i++)
  {
    int ijet = DR_lep[i];

    jet_lv.SetPtEtaPhiM(ak8_pt.at(ijet), ak8_eta.at(ijet), ak8_phi.at(ijet), ak8_mass.at(ijet));
    if (leptonicParticle == 0)
    {
      DR_LP_AK8 = jet_lv.DeltaR(Wlv);
      isLeptonic_W = true;
    }
    if (leptonicParticle == 1)
    {
      DR_LP_AK8 = jet_lv.DeltaR(top_lv);
      isLeptonic_t = true;
    }
    if (jets_lv.size() > 1)
      continue; // we only take 2 for single Bprime

    if (jet_lv.DeltaR(top_lv) > 0.8 and isLeptonic_t)
    {
      jets_lv.push_back(make_pair(jet_lv, ijet));
    }
    if (jet_lv.DeltaR(Wlv) > 0.8 and isLeptonic_W)
    {
      jets_lv.push_back(make_pair(jet_lv, ijet));
    }
  }

  float Bprime_mass = -999;
  float Bprime_pt = -999;
  float Bprime_eta = 9;
  float Bprime_phi = 9;
  float Bprime_DR = -9;
  float Bprime_ptbal = -999;
  float Bprime_chi2 = -999;

  TLorentzVector Wcand_lv;
  TLorentzVector Tcand_lv;
  TLorentzVector Bprime_lv;
  float validBDecay = -1;

  float taggedTjet = 0;
  float taggedTjet2nd = 0;
  float taggedWbjetJet = 0;
  float taggedWjet = 0;
  float taggedWjet2nd = 0;
  bool is2Jet = false;

  // ----------------------------------------------------------------------------
  // VLQ Decay -- 1 or 2 AK8 jets away from leptonic particle
  // ----------------------------------------------------------------------------
  if (jets_lv.size() > 2)
  {
    cout << "Problem: > 2 AK8s for Bprime reco" << endl;
  }
  else if (jets_lv.size() == 0)
  {
    cout << "Failed reco event, no jets" << endl;
  }

  int npass_reco = 0;

  if (jets_lv.size() == 1 || jets_lv.size() == 2)
  {
    npass_reco = npass_reco + 1;

    if (jets_lv.size() == 2)
    {
      is2Jet = true;
    }

    // get the tags
    float jet1_tag = dpak8_tag.at(jets_lv.at(0).second);
    float jet2_tag = -1;
    if (is2Jet)
      jet2_tag = dpak8_tag.at(jets_lv.at(1).second);

    // pair up the jet tag with the pT index 0,1,2 and sort by tag (orders J, T, W,)
    vector<pair<float, float>> decayJets;
    decayJets.push_back(make_pair(jet1_tag, 0));
    if (is2Jet)
      decayJets.push_back(make_pair(jet2_tag, 1));
    sort(decayJets.begin(), decayJets.end());

    // Start forming 2 particle groups
    validBDecay = -1;

    // Derived from Previous logic: TT -> tZ bW, BB -> tW bZ
    if (isLeptonic_W)
    {

      Wcand_lv = Wlv; // W candidate is always the leptonic W here

      if (decayJets.at(0).first == 1)
      { // leading jet is top tagged
        validBDecay = 1;
        taggedTjet = 1;
        Tcand_lv = jets_lv.at(decayJets.at(0).second).first; // decayJets.second gives the jets_lv index to get 4-vec
      }
      else if (is2Jet && decayJets.at(1).first == 1)
      {                  // 2nd-leading jet is top tagged
        validBDecay = 0; // 0 for suboptimal validity
        taggedTjet = 1;
        Tcand_lv = jets_lv.at(decayJets.at(1).second).first;
      }
      else
      { // leading jets are not top tagged
        if (!is2Jet)
        {
          validBDecay = -1;                                    // all other 1-jet events are really mistagged
          Tcand_lv = jets_lv.at(decayJets.at(0).second).first; // take the jet as the T candidate with a bad tag
        }
        else
        { // it's a 2-jet event, will assume partially-merged top and add the two jets, looking for a W tag
          if (decayJets.at(0).first == 2)
          { // W tag found
            validBDecay = 1;
            taggedWbjetJet = 1;
            Tcand_lv = jets_lv.at(decayJets.at(0).second).first + jets_lv.at(decayJets.at(1).second).first;
          }
          else if (decayJets.at(1).first == 2)
          {                  // W tag found
            validBDecay = 0; // sub-optimal
            taggedWbjetJet = 1;
            Tcand_lv = jets_lv.at(decayJets.at(0).second).first + jets_lv.at(decayJets.at(1).second).first;
          }
          else
          { // any other set of tags
            validBDecay = -1;
            Tcand_lv = jets_lv.at(decayJets.at(0).second).first + jets_lv.at(decayJets.at(1).second).first;
          }
        }
      }

      Bprime_DR = Wcand_lv.DeltaR(Tcand_lv);
      Bprime_ptbal = Wcand_lv.Pt() / Tcand_lv.Pt();
      Bprime_chi2 = pow(Tcand_lv.M() - 170.1, 2) / (14.29 * 14.29) + pow(Bprime_DR - TMath::Pi(), 2) / (0.2 * 0.2) + pow(Bprime_ptbal - 1, 2) / (0.8 * 0.8);
      // hadronic t chi2 with values from 2016 B2G-17-018 (W isn't there because W_lv uses mass constraint)
    }
    else
    { // isLeptonic_t

      Tcand_lv = top_lv; // T candidate is always the leptonic top here

      if (decayJets.at(0).first == 2)
      { // leading jet is W-tagged
        validBDecay = 1;
        taggedWjet = 1;
        Wcand_lv = jets_lv.at(decayJets.at(0).second).first;
      }
      else if (is2Jet && decayJets.at(1).first == 2)
      {                  // 2nd-leading jet is W-tagged
        validBDecay = 0; // sub-optimal
        taggedWjet = 1;
        Wcand_lv = jets_lv.at(decayJets.at(1).second).first;
      }
      else
      {
        validBDecay = -1;
        Wcand_lv = jets_lv.at(decayJets.at(0).second).first; // let's assume a 1-jet mistag scenario
      }

      Bprime_DR = Wcand_lv.DeltaR(Tcand_lv);
      Bprime_ptbal = Wcand_lv.Pt() / Tcand_lv.Pt();
      Bprime_chi2 = pow(Tcand_lv.M() - 172.6, 2) / (19.1 * 19.1) + pow(Wcand_lv.M() - 85.5, 2) / (8.7 * 8.7) + pow(Bprime_DR - TMath::Pi(), 2) / (0.2 * 0.2) + pow(Bprime_ptbal - 1, 2) / (0.8 * 0.8); // leptonic t, hadronic W chi2 with values from 2016 B2G-17-018
    }

    Bprime_lv = Wcand_lv + Tcand_lv;
    Bprime_mass = Bprime_lv.M();
    Bprime_pt = Bprime_lv.Pt();
    Bprime_eta = Bprime_lv.Eta();
    Bprime_phi = Bprime_lv.Phi();
  }

  // Include validBDecy_DeepAK8 variable and what it's decayed as
  // leptonic and hadronic jetIDs can be slimmed out, no longer exist
  RVec<float> BPrimeVec = {Bprime_mass, Bprime_pt, Bprime_eta, Bprime_phi, Bprime_DR, Bprime_ptbal, Bprime_chi2, validBDecay, taggedWbjetJet, taggedTjet, taggedWjet};
  return BPrimeVec;
}
