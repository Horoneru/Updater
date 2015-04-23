#include "updater.h"
#include <iostream>

Updater::Updater(QString appName, QString currentVersion, QString urlLatestVersion, QString urlLatestExe, QString showIfNoUpdate, QObject *parent) :
    QObject(parent), a_urlVersion(urlLatestVersion), a_urlExe(urlLatestExe), a_appName(appName), a_currentAppVersion(currentVersion)
{
    if(showIfNoUpdate == "show")
        a_showIfNoUpdate = true;
    else
        a_showIfNoUpdate = false;

    a_netManager = new QNetworkAccessManager(this);
}

void Updater::start()
{
    if(a_netManager->networkAccessible() != QNetworkAccessManager::NotAccessible)
    {
        qDebug() << "starting";
        checkUpdate();
    }
    else
        noNetworkError();
}

void Updater::checkUpdate()
{
    a_netRequest.setUrl(a_urlVersion);
    a_netReply = a_netManager->get(a_netRequest);
    connect(a_netReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(errorHandling(QNetworkReply::NetworkError)));
    connect(a_netReply, SIGNAL(finished()), this, SLOT(processUpdateReading()));
    a_checkUpdateProgressDialog = new QProgressDialog (tr("Vérification des mises à jour..."), QString(), 0, 0);
    a_checkUpdateProgressDialog->setWindowTitle("Updater");
    a_progressTimer.setSingleShot(true);
    a_progressTimer.start(1000); //This delays the dialog
    connect(&a_progressTimer, SIGNAL(timeout()), this, SLOT(showCheckingProgress()));
}

void Updater::downloadUpdate()
{
    if(a_netManager->networkAccessible() != QNetworkAccessManager::NotAccessible)
    {
        a_progressDialog = new QProgressDialog(nullptr);
        a_progressDialog->setWindowModality(Qt::ApplicationModal);
        connect(a_progressDialog, SIGNAL(canceled()),this, SLOT(cancelDownload()));
        a_progressDialog->setWindowTitle(tr("Veuillez patienter"));
        a_progressDialog->setLabelText(tr("Téléchargement de la mise à jour de %1 en cours").arg(a_appName));
        a_progressDialog->setValue(0);
        a_progressDialog->show();
        a_netRequest.setUrl(a_urlExe);
        a_netReply = a_netManager->get(a_netRequest);
        connect(a_netReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(errorHandling(QNetworkReply::NetworkError)));
        connect(a_netReply, SIGNAL(finished()),this, SLOT(writeUpdate()));
        connect(a_netReply, SIGNAL(downloadProgress(qint64, qint64)),this, SLOT(updateProgress(qint64, qint64)));
    }
    else
        noNetworkError();
}

void Updater::noNetworkError()
{
    QMessageBox::critical(nullptr, tr("Erreur ! "), tr("Impossible d'accéder au réseau"));
    exit(1);
}

void Updater::writeUpdate()
{
    if(a_netReply->bytesAvailable() != 0)
    {
        QFileInfo file(a_urlExe);
        QString executableName = file.fileName();
        if(QFile::exists(executableName))
        {
            if(QFile::exists("_old_" + executableName))
                QFile::remove("_old_" + executableName);
            QFile::rename(executableName, "_old_" + executableName);
        }
        qDebug() << "writing" << a_netReply->bytesAvailable();
        QFile fileToWrite (QFileInfo(a_urlExe).fileName(), this);
        if(fileToWrite.open(QFile::WriteOnly | QIODevice::Truncate))
        {
            fileToWrite.write(a_netReply->readAll());
            fileToWrite.close();
            QMessageBox endBox (nullptr);
            endBox.setWindowTitle(tr("Mise à jour terminée ! "));
            endBox.setText(tr("La mise à jour %1 a pu être appliquée avec succès\nVoulez-vous lancer à nouveau %2?").arg(a_newAppVersion).arg(a_appName));
            endBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            endBox.setDefaultButton(QMessageBox::Yes);
            endBox.setButtonText(QMessageBox::Yes, tr("Relancer %1").arg(a_appName));
            endBox.setButtonText(QMessageBox::No, tr("Pas maintenant"));
            int reply = endBox.exec(); //Awaiting user reply
            if(reply == QMessageBox::Yes)
                QDesktopServices::openUrl(QUrl::fromLocalFile(executableName));
            exit(0);
        }
        else
        {
            qDebug() << fileToWrite.errorString();
            QMessageBox::critical(nullptr, tr("Erreur"), tr("La mise à jour n'a pas pu être écrite : ") + fileToWrite.errorString());
            exit(1);
        }
    }

}

void Updater::cancelDownload()
{
    if(a_checkUpdateProgressDialog->isVisible())
        a_checkUpdateProgressDialog->cancel();
    if(a_progressDialog->isVisible())
        a_progressDialog->cancel();
    exit(0);
}

void Updater::showCheckingProgress()
{
    a_checkUpdateProgressDialog->show();
}

void Updater::errorHandling(QNetworkReply::NetworkError errorType)
{
    QMessageBox::critical(nullptr, tr("Erreur"), tr("La mise à jour n'a pas pu être récupérée : ") + a_netReply->errorString());
    qDebug() << a_netReply->errorString();
    exit(1);
}

void Updater::processUpdateReading()
{
    a_progressTimer.stop();// Those avoid the checkupdateprogress to be shown if we're reading the update really fast
    a_checkUpdateProgressDialog->close();//  ^
    if(a_netReply->bytesAvailable() != 0)
    {
        a_newAppVersion = a_netReply->readAll();
        if(a_newAppVersion != a_currentAppVersion) //Needs to update
        {
            QMessageBox box (nullptr);
            box.setWindowTitle(tr("Mise à jour disponible ! "));
            box.setText(tr("La mise à jour %1 pour %2 est disponible.\nVersion actuelle : %3\nVoulez-vous télécharger la mise à jour ?").arg(a_newAppVersion).arg(a_appName).arg(a_currentAppVersion));
            box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            box.setDefaultButton(QMessageBox::No);
            box.setButtonText(QMessageBox::No, tr("Pas maintenant"));
            box.setButtonText(QMessageBox::Yes, tr("Oui"));
            int reply = box.exec();
            if (reply == QMessageBox::Yes)
            {
                qDebug() << "okay";
                std::cout << "downloading" << std::endl; //Reporting back from the original app process that we're downloading, so it closes itself
                downloadUpdate();
            }
            else
                exit(0);
        }
        else //Up to date, we're good to exit now
        {
            if(a_showIfNoUpdate)
                QMessageBox::information(nullptr, tr("Aucune mise à jour disponible"), tr("Aucune mise à jour n'est disponible pour %1").arg(a_appName));
            exit(0);
        }

    }
    else
        qDebug () << "something bad happened";
}

void Updater::updateProgress(qint64 bytesWritten, qint64 bytesTotal)
{
    a_progressDialog->setMaximum(bytesTotal);
    a_progressDialog->setValue(bytesWritten);
    if(a_progressDialog->value() == bytesTotal)
        a_progressDialog->done(0);
}

Updater::~Updater()
{
}
