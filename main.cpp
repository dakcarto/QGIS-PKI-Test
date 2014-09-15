#include <QApplication>
#include "webpage.h"

int main( int argc, char ** argv )
{
  // start application
  QApplication app( argc, argv, TRUE );

  // load default widget
  WebPage * mWebPage = new WebPage();
  mWebPage->show();
  mWebPage->raise();
  mWebPage->activateWindow();

  // start application event loop
  return app.exec();
}
