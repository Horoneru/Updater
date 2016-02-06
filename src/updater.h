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
    Updater(QString appName, QString currentVersion, QString urlLatestVersion,
            QString urlLatestExe, QString showIfNoUpdate, QObject * parent = 0);
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
    QTimer m_progressTimer;
    QProgressDialog *m_checkUpdateProgressDialog;
    QProgressDialog *m_progressDialog;
    QNetworkAccessManager *m_netManager;
    QNetworkRequest m_netRequest;
    QNetworkReply *m_netReply;
    //The two urls for the updater to update itself when needed
    QString m_urlUpdaterVersion;
    QString m_urlUpdaterExe;
    //The actual infos from the application we're trying to update
    QString m_urlVersion;
    QString m_urlExe;
    QString m_appName;
    QString m_currentAppVersion;
    QString m_newAppVersion;
    bool m_newUpdaterVersion;
    bool m_showIfNoUpdate;
    QString m_currentUpdaterVersion;

    int m_idOs;
};
#endif // UPDATER_H
