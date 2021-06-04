#include "ConverterPanel.hxx"


void runConverterPanel()
{
   auto panel = new ConverterPanel();

/*   TH1F *hpx = new TH1F("hpx","This is the px distribution",100,-4,4);
   hpx->FillRandom("gaus", 10000);
   hpx->Draw();
*/
   panel->Show();
}

