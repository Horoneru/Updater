#include "updater.h"
#include <iostream>

Updater::Updater(QString appName, QString currentVersion, QString urlLatestVersion,
                 QString urlLatestExe, QString showIfNoUpdate, QObject *parent) :
    QObject(parent), m_urlVersion(urlLatestVersion), m_urlExe(urlLatestExe),
    m_appName(appName), m_currentAppVersion(currentVersion)
{
    if(showIfNoUpdate == "show")
        m_showIfNoUpdate = true;
    else
        m_showIfNoUpdate = false;

    m_netManager = new QNetworkAccessManager(this);
}

void Updater::start()
{
    if(m_netManager->networkAccessible() != QNetworkAccessManager::NotAccessible)
    {
        qDebug() << "starting";
        checkUpdate();
    }
    else
        noNetworkError();
}

void Updater::checkUpdate()
{
    m_netRequest.setUrl(m_urlVersion);
    m_netReply = m_netManager->get(m_netRequest);

    connect(m_netReply, SIGNAL(error(QNetworkReply::NetworkError)), this,
            SLOT(errorHandling(QNetworkReply::NetworkError)));
    connect(m_netReply, SIGNAL(finished()), this, SLOT(processUpdateReading()));

    m_checkUpdateProgressDialog = new QProgressDialog
            (tr("Vérification des mises à jour..."), QString(), 0, 0);
    m_checkUpdateProgressDialog->setWindowTitle("Updater");
    m_progressTimer.setSingleShot(true);
    m_progressTimer.start(1000); //This delays the dialog

    connect(&m_progressTimer, SIGNAL(timeout()), this, SLOT(showCheckingProgress()));
}

void Updater::downloadUpdate()
{
    if(m_netManager->networkAccessible() != QNetworkAccessManager::NotAccessible)
    {
        m_progressDialog = new QProgressDialog(nullptr);
        m_progressDialog->setWindowModality(Qt::ApplicationModal);

        m_progressDialog->setWindowTitle(tr("Veuillez patienter"));
        m_progressDialog->setLabelText(tr("Téléchargement de la mise à jour de %1 en cours").arg(m_appName));
        m_progressDialog->setValue(0);
        connect(m_progressDialog, SIGNAL(canceled()),this, SLOT(cancelDownload()));
        m_progressDialog->show();

        m_netRequest.setUrl(m_urlExe);
        m_netReply = m_netManager->get(m_netRequest);

        connect(m_netReply, SIGNAL(error(QNetworkReply::NetworkError)), this,
                SLOT(errorHandling(QNetworkReply::NetworkError)));
        connect(m_netReply, SIGNAL(finished()),this, SLOT(writeUpdate()));
        connect(m_netReply, SIGNAL(downloadProgress(qint64, qint64)),this,
                SLOT(updateProgress(qint64, qint64)));
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
    if(m_netReply->bytesAvailable() != 0)
    {
        QFileInfo file(m_urlExe);
        QString executableName = file.fileName();

        if(QFile::exists(executableName))
        {
            if(QFile::exists("_old_" + executableName))
                QFile::remove("_old_" + executableName);
            QFile::rename(executableName, "_old_" + executableName);
        }
        QFile fileToWrite (QFileInfo(m_urlExe).fileName(), this);
        if(fileToWrite.open(QFile::WriteOnly | QIODevice::Truncate))
        {
            fileToWrite.write(m_netReply->readAll());
            fileToWrite.close();

            QMessageBox endBox (nullptr);
            endBox.setWindowTitle(tr("Mise à jour terminée ! "));
            endBox.setText(tr("La mise à jour %1 a pu être appliquée avec succès\n"
                              "Voulez-vous lancer à nouveau %2?").arg(m_newAppVersion).arg(m_appName));
            endBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            endBox.setDefaultButton(QMessageBox::Yes);
            endBox.setButtonText(QMessageBox::Yes, tr("Relancer %1").arg(m_appName));
            endBox.setButtonText(QMessageBox::No, tr("Pas maintenant"));
            int reply = endBox.exec(); //Awaiting user reply
            if(reply == QMessageBox::Yes)
                QDesktopServices::openUrl(QUrl::fromLocalFile(executableName));
            exit(0);
        }
        else
        {
            qDebug() << fileToWrite.errorString();
            QMessageBox::critical(nullptr, tr("Erreur"),
                                  tr("La mise à jour n'a pas pu être écrite : ") +
                                  fileToWrite.errorString());
            exit(1);
        }
    }

}

void Updater::cancelDownload()
{
    if(m_checkUpdateProgressDialog->isVisible())
        m_checkUpdateProgressDialog->cancel();
    if(m_progressDialog->isVisible())
        m_progressDialog->cancel();
    exit(0);
}

void Updater::showCheckingProgress()
{
    m_checkUpdateProgressDialog->show();
}

void Updater::errorHandling(QNetworkReply::NetworkError errorType)
{
    Q_UNUSED(errorType);
    QMessageBox::critical(nullptr, tr("Erreur"),
                          tr("La mise à jour n'a pas pu être récupérée : ") +
                          m_netReply->errorString());
    qDebug() << m_netReply->errorString();
    exit(1);
}

void Updater::processUpdateReading()
{
    m_progressTimer.stop();// Those avoid the checkupdateprogress to be shown if we're reading the update really fast
    m_checkUpdateProgressDialog->close();//  ^

    if(m_netReply->bytesAvailable() != 0)
    {
        m_newAppVersion = m_netReply->readAll();
        if(m_newAppVersion != m_currentAppVersion) //Needs to update
        {
            QMessageBox box (nullptr);
            box.setWindowTitle(tr("Mise à jour disponible ! "));
            box.setText(tr("La mise à jour %1 pour %2 est disponible.\n"
                           "Version actuelle : %3\n"
                           "Voulez-vous télécharger la mise à jour ?")
                        .arg(m_newAppVersion)
                        .arg(m_appName)
                        .arg(m_currentAppVersion));
            box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            box.setDefaultButton(QMessageBox::No);
            box.setButtonText(QMessageBox::No, tr("Pas maintenant"));
            box.setButtonText(QMessageBox::Yes, tr("Oui"));
            int reply = box.exec();
            if (reply == QMessageBox::Yes)
            {
                //Reporting back from the original app process that we're downloading, so it closes itself
                std::cout << "downloading" << std::endl;
                downloadUpdate();
            }
            else
                exit(0);
        }
        else //Up to date, we're good to exit now
        {
            if(m_showIfNoUpdate)
                QMessageBox::information(nullptr, tr("Aucune mise à jour disponible"),
                                         tr("Aucune mise à jour n'est disponible pour %1").arg(m_appName));
            exit(0);
        }

    }
    else
        qDebug () << "no reply";
}

void Updater::updateProgress(qint64 bytesWritten, qint64 bytesTotal)
{

    if(m_progressDialog->maximum() == 100) //If maximum not set yet
        m_progressDialog->setMaximum(bytesTotal);

    m_progressDialog->setValue(bytesWritten);
    if(m_progressDialog->value() == bytesTotal)
        m_progressDialog->done(0);
}

Updater::~Updater()
{}
