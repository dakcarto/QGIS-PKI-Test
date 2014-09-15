/***************************************************************************
    main.cpp  -  test app for PKI integration in QGIS
                             -------------------
    begin                : 2014-09-12
    copyright            : (C) 2014 by Boundless Spatial, Inc.
    web                  : http://boundlessgeo.com
    author               : Larry Shaffer
    email                : larrys at dakotacarto dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
