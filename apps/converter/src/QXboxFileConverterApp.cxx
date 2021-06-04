#include <QApplication>
#include "QXboxFileConverter.hxx"

int main(int argc, char *argv[]) {
    
  QApplication app(argc, argv);  
  QXboxFileConverter window;
  
  window.resize(450, 700);
  window.setWindowTitle("Xbox Tdms File Converter");
  window.show();

  return app.exec();
}
