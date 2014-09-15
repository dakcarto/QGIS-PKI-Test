QGIS-PKI-Test
=============

Reference/test app to help with PKI integration in QGIS.

Following these instructions and building/running this test app will ensure your
PKI setup on the backend server is accessible from a Qt-based application.

Once you have verified this access works, i.e. your backend server is working
fine with a client-supplied SSL certificate/key, test the integration in QGIS.

Set up test components
----------------------

Initial test revolves around connecting to an `OpenGeo Suite GeoServer`_
backend that has PKI enabled for a single user named 'rod'. See GeoServer
community docs for info on `Configuring X.509 Certificate Authentication`_.

Configure GeoServer before continuing.

.. note:: This test app *will* work with other PKI backends. You will have to
   configure them, then edit `webpage.cpp` in the test app's source tree, before
   building, and customize the `*.pem` file names relative to your Certificate
   Authority and client cert/key. Then, when testing in the app, just paste in
   your URL and hit return/enter.

Create test folder structure
............................

Create a project folder for testing PKI, e.g. named `PKI-Test`::

  $ cd ~ && mkdir "PKI-Test" && cd "PKI-Test"

Inside the `PKI-Test` the following folders::

  $ mkdir PKI QGIS-PKI-Test-build

Pre-process the sample certificates and private key
...................................................

Download the PKI `sample certificates and private key`_.

Split and convert, to PEM format, the client certificate/key (note
password for all commands is *password*)::

  # split DER-formatted sample cert/key into separate PEM files
  $ openssl pkcs12 -in rod.p12 -clcerts -nokeys -out rod_cert.pem
  $ openssl pkcs12 -in rod.p12 -nocerts -nodes -out rod_key.pem

.. note:: Since I am testing on a Mac, I also converted the `ca.pem` certificate
   authority file and changed the line endings from Windows (CRLF) to Unix (LF).
   However, the test app (i.e. underlying QtNetwork module) appears to handle
   the X.509 components with either line ending formats::

     $ mv ca.pem ca-crlf.pem
     $ tr -d '\15\32' < ca-crlf.pem  > ca.pem

   *If you are testing on Windows, you do NOT need to make this change.*

Copy the ``ca.pem``, ``rod_cert.pem`` and ``rod_key.pem`` files to the folder
``~/PKI-Test/PKI``.

Validate the conversion was successful
......................................

::

  $ cd ~/PKI-Test/PKI
  $ openssl rsa -noout -modulus -in rod_key.pem
  $ openssl x509 -noout -modulus -in rod_cert.pem

The resultant modulus for both files should match, and probably looks like::

  Modulus=AEE99872F94ECD2D1DA130443C7CCC71EDF298DA0C02F822C211F4D9D88E48A4CEC160
  D915422AFDB2AC6972E0204192FC8D3CA7DB40DC6A095AA0DFD9EA336CB87BAB3102998C78485E
  31C4C15AD09079F5AC73B038E63D993B4A3D6D3FBA1EA44515011C7273A2F7385302D392DC3A1C
  084D54947105C43C8332150B06D573

Use `openssl` test client to access GeoServer
.............................................

::

  $ cd ~/PKI-Test/PKI
  $ echo "GET /" | openssl s_client -connect localhost:8443 \
    -cert 'rod_cert.pem' -key 'rod_key.pem' -CAfile 'ca.pem'

.. note:: The second command is a *single* line, wrapped to two.

For a successful connection test, it should look something like::

  <bunch of certificate listings>
  ---
  SSL handshake has read 12433 bytes and written 2110 bytes
  ---
  New, TLSv1/SSLv3, Cipher is EDH-RSA-DES-CBC3-SHA
  Server public key is 1024 bit
  Secure Renegotiation IS supported
  Compression: NONE
  Expansion: NONE
  SSL-Session:
      Protocol  : TLSv1
      Cipher    : EDH-RSA-DES-CBC3-SHA
      Session-ID: <some hash>
      Session-ID-ctx:
      Master-Key: <some hash>
      Key-Arg   : None
      Start Time: 1410803288
      Timeout   : 300 (sec)
      Verify return code: 0 (ok)
  ---
  DONE

.. note:: The self-signed client certificate used here will reference the
   'Spring Security Test CA'.

Set up and build QGIS-PKI-Test app
----------------------------------

Dependencies
............

You will need, at a minimum, software for building from source code for your OS,
`CMake`_ and `Qt libraries`_. Optionally, you can install `git`_ to clone the
source code from the repository.

Get source code and build
.........................

Clone source code from github.com::

  $ cd ~/PKI-Test
  $ git clone git@github.com:dakcarto/QGIS-PKI-Test.git

This will create a folder named 'QGIS-PKI-Test' with the source code in it.

.. note:: If you are not using `git`, you can download the source code and
   expand it archive. Once decompressed, change the name of the archive to
   'QGIS-PKI-Test', e.g.::

     $ cd ~/PKI-Test
     $ wget "https://codeload.github.com/dakcarto/QGIS-PKI-Test/zip/master" -O "QGIS-PKI-Test-master.zip"
     # OR, on Mac (no wget installed by default):
     $ curl "https://codeload.github.com/dakcarto/QGIS-PKI-Test/zip/master" -o "QGIS-PKI-Test-master.zip"
     $ unzip QGIS-PKI-Test-master.zip
     $ mv QGIS-PKI-Test-master QGIS-PKI-Test

Build the app (no need to install):

CMake supports out-of-source directory building::

  $ ~/PKI-Test/QGIS-PKI-Test-build
  $ cmake ../QGIS-PKI-Test
  $ make

This should result in a binary app named `QGIS-PKI-Test`.

Run the `QGIS-PKI-Test` app::

  $ cd ~/PKI-Test/QGIS-PKI-Test-build
  $ ./QGIS-PKI-Test

The test app is a simple Web browser. Test the last URL from the location bar
drop down menu, e.g. the one that starts with::

  https://localhost:8443/geoserver/opengeo/wms?service=WMS&version=1.1.0...

A successful connection to GeoServer should `look like this`_.

Some notes concerning test app and integration
----------------------------------------------

- Since Qt is built against OpenSSL_, the first implementation will work solely
  with the OpenSSL cert/key stores, i.e. no direct access to the underlying
  platform-native store on Windows or Mac.

- The `ca.pem` file is necessary to add to the SSL configuration, otherwise the
  trust chain can not be built. This should be a global app option in QGIS.

- The `QSslError::SelfSignedCertificate` error has been added as a default
  expected SSL error, since in the case of GeoServer's `ca.pem` the CA is
  self-signed. This should probably be a default expected SSL error in QGIS as
  well.

- The `QWebView` (from Qt 4.8.6) used in this example did not forward the client
  cert/key on to the peer during SSL handshake, when using its embedded
  `QNetworkAccessManager`. The manager was used to separately access the content
  and then load the result into the `QWebView`. This may result in some web
  pages not loading completely and possibly lead to crashes unrelated to the
  actual PKI testing.

  In other words, use this browser only for testing connections to test servers,
  ideally local ones.

- This test app does not support multi-threaded Web browsing, i.e. the app's GUI
  will occasionally freeze when accessing pages, until all of their components
  are downloaded and ready to display. This is a limitation of Qt's `QWebView`
  widget, when used in a basic GUI layout, and (probably) *not* a reflection
  upon the backend's responsiveness.

.. _OpenSSL: https://www.openssl.org/
.. _OpenGeo Suite GeoServer: http://boundlessgeo.com/solutions/solutions-software/geoserver/
.. _Configuring X.509 Certificate Authentication: http://suite.opengeo.org/opengeo-docs/geoserver/security/tutorials/cert/index.html
.. _sample certificates and private key: http://suite.opengeo.org/opengeo-docs/geoserver/_downloads/sample_certs.zip
.. _CMake: http://www.cmake.org/download/
.. _Qt libraries: http://qt-project.org/downloads/
.. _git: http://git-scm.com/downloads/
.. _look like this: http://drive.dakotacarto.com/qgis/geoserver-pki-access.png
