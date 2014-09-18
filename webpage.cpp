/***************************************************************************
    webpage.cpp  -  test app for PKI integration in QGIS
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

#include "webpage.h"
#include "ui_webpage.h"

#include <QDir>
#include <QInputDialog>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslError>
#include <QSslKey>

WebPage::WebPage( QWidget *parent ) :
  QDialog( parent ),
  mNaMan( 0 ),
  mReply( 0 ),
  mLoaded( false )
{
  setupUi(this);

  comboBox->lineEdit()->setAlignment( Qt::AlignLeft );

  QStringList urlList;
  urlList << "http://localhost"
          << QString( "http://localhost:8080" )
          << QString( "https://localhost:8443" )
          << QString( "http://www.google.com" )
          << QString( "https://localhost:8443/geoserver/opengeo/wms?service=WMS&version=1.1.0"
                      "&request=GetMap&layers=opengeo:countries&styles=&bbox=-180.0,-90.0,180.0,90.0"
                      "&width=720&height=400&srs=EPSG:4326&format=application/openlayers" );

  comboBox->addItems( urlList );

  mNaMan = webView->page()->networkAccessManager();

  connect( mNaMan, SIGNAL( finished( QNetworkReply*) ), this, SLOT( requestReply( QNetworkReply* ) ) );
  connect(
    mNaMan, SIGNAL( sslErrors( QNetworkReply*, const QList<QSslError>& ) ),
    this, SLOT( onSslErrors( QNetworkReply*, const QList<QSslError>& ) )
  );

  connect( webView, SIGNAL( linkClicked( QUrl ) ), this, SLOT( loadUrl( QUrl ) ) );
  connect( webView, SIGNAL( urlChanged( QUrl ) ), this, SLOT( setLocation( QUrl ) ) );
  connect( webView, SIGNAL( titleChanged( const QString& ) ), SLOT( updateTitle( const QString& ) ) );

  connect( backButton, SIGNAL( clicked() ), webView, SLOT( back() ) );
  connect( forwardButton, SIGNAL( clicked() ), webView, SLOT( forward() ) );
  connect( reloadButton, SIGNAL( clicked() ), webView, SLOT( reload() ) );
  connect( stopButton, SIGNAL( clicked() ), webView, SLOT( stop() ) );

//  connect( comboBox->lineEdit(), SIGNAL( returnPressed() ), this, SLOT( loadUrl() ) );
  connect( comboBox, SIGNAL( activated( const QString& ) ), this, SLOT( loadUrl( const QString& ) ) );
  connect( clearButton, SIGNAL( clicked() ), this, SLOT( clearLog() ) );

}

WebPage::~WebPage()
{
}

void WebPage::updateTitle( const QString& title )
{
  setWindowTitle( title );
}

void WebPage::setLocation( const QUrl& url )
{
  comboBox->lineEdit()->setText( url.toString() );
}

void WebPage::appendLog( const QString& msg )
{
  plainTextEdit->appendPlainText( msg );
}

void WebPage::clearLog()
{
  plainTextEdit->clear();
}

void WebPage::showEvent( QShowEvent * e )
{
  if ( !mLoaded )
  {
    loadUrl();
    mLoaded = true;
  }

  QDialog::showEvent( e );
}

void WebPage::loadUrl( const QString& url )
{
  QString curText( comboBox->lineEdit()->text() );
  if ( url.isEmpty() && curText.isEmpty() )
  {
    return;
  }

  QUrl reqUrl( url.isEmpty() ? comboBox->lineEdit()->text() : url );
  loadUrl( reqUrl );
}

void WebPage::loadUrl( const QUrl& url )
{
  if ( url.isEmpty() || !url.isValid() )
  {
    return;
  }

  QNetworkRequest req;
  req.setUrl( url );
  req.setRawHeader("User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.9; rv:32.0) Gecko/20100101 Firefox/32.0" );

  if ( url.scheme().toLower() == "https" )
  {
    bool ok;
    QByteArray passphrase;
    QString text = QInputDialog::getText( this, tr( "Enter Private Key Passphrase" ),
                                          tr( "Passphrase:" ), QLineEdit::PasswordEchoOnEdit,
                                          "", &ok );
    if ( ok && !text.isEmpty() )
    {
       passphrase = text.toLatin1();
    }
//    else
//    {
//      return;
//    }

    QSslConfiguration sslConfig( req.sslConfiguration() );
    //QSslConfiguration sslConfig( QSslConfiguration::defaultConfiguration() );

    sslConfig.setProtocol( QSsl::TlsV1SslV3 );

    // add GeoServer test CA
    QSslCertificate ca( certAuth() );
    //appendLog( QString( "CertAuth is null: %1" ).arg( ca.isNull() ? "true" : "false" ) );
    //appendLog( QString( "CertAuth is valid: %1" ).arg( ca.isValid() ? "true" : "false" ) );
    QList<QSslCertificate> sslCAs( sslConfig.caCertificates() );
    //QList<QSslCertificate> sslCAs;
    sslCAs << ca;
    sslConfig.setCaCertificates( sslCAs );

    // add GeoServer test client cert and key
    QSslCertificate cc( clientCert() );
    //appendLog( QString( "ClientCert is null: %1" ).arg( cc.isNull() ? "true" : "false" ) );
    //appendLog( QString( "ClientCert is valid: %1" ).arg( cc.isValid() ? "true" : "false" ) );

    sslConfig.setLocalCertificate( cc );

    QSslKey ck( clientKey( passphrase ) );
    //appendLog( QString( "CertKey is null: %1" ).arg( ck.isNull() ? "true" : "false" ) );

    sslConfig.setPrivateKey( ck );

    req.setSslConfiguration( sslConfig );
  }

  //webView->load( req ); // hey, why doesn't this work? doesn't pass ssl cert/key

  delete mReply;
  mReply = 0;

  mReply = mNaMan->get( req );
  mReply->ignoreSslErrors( expectedSslErrors() );
  clearWebView();
  setLocation( mReply->request().url() );
  webView->setFocus();
  connect( mReply, SIGNAL( readyRead() ), this, SLOT( loadReply() ) );
}

void WebPage::loadReply()
{
  QUrl url( mReply->url() );
  webView->setContent( mReply->readAll(), QString(), url );
}

void WebPage::clearWebView()
{
  webView->setContent( 0 );
}

void WebPage::requestReply( QNetworkReply * reply )
{
  if ( reply->error() != QNetworkReply::NoError )
  {
    appendLog( QString( "Network error #%1: %2" ).arg( reply->error() ).arg( reply->errorString() ) );
  }
}

void WebPage::onSslErrors( QNetworkReply* reply, const QList<QSslError>& errors )
{
//  reply->ignoreSslErrors( expectedSslErrors() );

  QString msg = QString( "SSL errors occured accessing URL %1:" ).arg( reply->request().url().toString() );

  foreach ( const QSslError& error, errors )
  {
    if ( error.error() == QSslError::NoError )
      continue;

    msg += "\n" + error.errorString();
  }

  appendLog( msg );
}

QString WebPage::pkiDir()
{
  return QDir( qApp->applicationDirPath() + "/../PKI").absolutePath() + QDir::separator();
}

QSslCertificate WebPage::certAuth()
{
  QString selfSignedCert( pkiDir() + "ca.pem" );
  QList<QSslCertificate> cert = QSslCertificate::fromPath( selfSignedCert );
  return cert.at( 0 );
}

QSslCertificate WebPage::clientCert()
{
  QFile file( pkiDir() + "rod_cert.pem" );
  file.open( QIODevice::ReadOnly );
  QSslCertificate clientCert( &file );
  file.close();
  return clientCert;
}

QSslKey WebPage::clientKey( const QByteArray& passphrase )
{
  QFile file( pkiDir() + "rod_key_pass.pem" );
  file.open( QIODevice::ReadOnly );
  QSslKey clientKey( &file, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, passphrase );
  file.close();
  return clientKey;
}

QList<QSslError> WebPage::expectedSslErrors()
{
  QSslError error( QSslError::SelfSignedCertificate, certAuth() );
  QList<QSslError> expectedSslErrors;
  expectedSslErrors.append( error );
  return expectedSslErrors;
}
