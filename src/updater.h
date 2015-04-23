#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QMessageBox>
#include <QNetworkReply>
#include <QFile>
#include <QDebug>
#include <QByteArray>
#include <QProgressDialog>
#include <QCoreApplication>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QTimer>

class Updater : public QObject
{
    Q_OBJECT

public:
    enum OS { Windows, Linux, Mac };
    Updater(QString appName, QString currentVersion, QString urlLatestVersion, QString urlLatestExe, QString showIfNoUpdate, QObject * parent = 0);
    ~Updater();
    void start();

private slots:
    void errorHandling(QNetworkReply::NetworkError errorType);
    void processUpdateReading();
    void updateProgress(qint64 bytesWritten , qint64 bytesTotal);
    void writeUpdate();
    void cancelDownload();
    void showCheckingProgress();

private:
    void checkUpdate();
    void downloadUpdate();
    void noNetworkError();

    //Attributes
    QTimer a_progressTimer;
    QProgressDialog *a_checkUpdateProgressDialog;
    QProgressDialog *a_progressDialog;
    QNetworkAccessManager *a_netManager;
    QNetworkRequest a_netRequest;
    QNetworkReply *a_netReply;
    //The two urls for the updater to update itself when needed
    QString a_urlUpdaterVersion;
    QString a_urlUpdaterExe;
    //The actual infos from the application we're trying to update
    QString a_urlVersion;
    QString a_urlExe;
    QString a_appName;
    QString a_currentAppVersion;
    QString a_newAppVersion;
    bool a_newUpdaterVersion;
    bool a_showIfNoUpdate;
    QString a_currentUpdaterVersion;

    int a_idOs;
};
#endif // UPDATER_H
