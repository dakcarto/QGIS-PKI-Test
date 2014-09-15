#ifndef WEBPAGE_H
#define WEBPAGE_H

#include "ui_webpage.h"

#include <QDialog>
#include <QNetworkAccessManager>
#include <QSslCertificate>


class WebPage : public QDialog, private Ui::WebPage
{
  Q_OBJECT

  public:
    explicit WebPage( QWidget *parent = 0 );
    ~WebPage();

  protected:
    void showEvent( QShowEvent * );

  private slots:
    void requestReply( QNetworkReply* reply );
    void onSslErrors( QNetworkReply* reply, const QList<QSslError>& errors );
    void updateTitle( const QString& title );
    void setLocation( const QUrl& url );
    void loadUrl( const QString& url = QString() );
    void loadUrl( const QUrl& url );
    void setWebViewHTML( const QString& html );
    void loadReply();
    void clearWebView();
    void clearLog();

  private:
    void appendLog( const QString& msg );
    QSslCertificate certAuth();
    QSslCertificate clientCert();
    QSslKey clientKey();
    QList<QSslError> expectedSslErrors();

    QNetworkAccessManager *mNaMan;
    QNetworkReply *mReply;
    bool mLoaded;
};

#endif // WEBPAGE_H
