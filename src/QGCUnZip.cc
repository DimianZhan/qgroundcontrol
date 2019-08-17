/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "QGCUnzip.h"

#include <zlib.h>
#include "unzip.h"

#include <QDebug>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

QString QGCUnzipSingleFile(const QString& unzipFilename, QString& errorString)
{
    const size_t cBuffer = 8192;

    unz_global_info64   gi;
    int                 unzError = UNZ_OK;
    char                filenameInZip[256];
    void*               buffer = nullptr;
    QFile               decompressedFile;
    int                 cBytesRead;

    // Determine temp location for unzipped file
    QString unzippedFilePath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (unzippedFilePath.isEmpty()) {
        unzippedFilePath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
        if (unzippedFilePath.isEmpty()) {
            errorString = QT_TR_NOOP("Unabled to find writable download location for unzipped file. Tried downloads and temp directory.");
            return QString();
        }
    }
    QDir unzippedFileDir(unzippedFilePath);

    unzFile unzipFile = unzOpen(unzipFilename.toLocal8Bit().data());
    if (!unzipFile) {
        errorString = QStringLiteral("unzOpen failed");
        goto Cleanup;
    }

    unzError = unzGetGlobalInfo64(unzipFile, &gi);
    if (unzError != UNZ_OK) {
        errorString = QStringLiteral("unzGetGlobalInfo64 failed %1").arg(unzError);
        goto Cleanup;
    }

    if (gi.number_entry != 1) {
        errorString = QStringLiteral("Zip file contains more than one file");
        goto Cleanup;
    }

    unz_file_info64 file_info;
    unzError = unzGetCurrentFileInfo64(unzipFile, &file_info, filenameInZip, sizeof(filenameInZip), nullptr, 0, nullptr, 0);
    if (unzError != UNZ_OK) {
        errorString = QStringLiteral("unzGetCurrentFileInfo64 failed %1").arg(unzError);
        goto Cleanup;
    }

    unzError = unzOpenCurrentFile(unzipFile);
    if (unzError != UNZ_OK) {
        errorString = QStringLiteral("unzOpenCurrentFile failed %1").arg(unzError);
        goto Cleanup;
    }

    decompressedFile.setFileName(unzippedFileDir.filePath(filenameInZip));
    if (!decompressedFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        errorString = QStringLiteral("Decompressed file open failed %1").arg(decompressedFile.errorString());
        goto Cleanup;
    }

    buffer = malloc(cBuffer);
    do {
        cBytesRead = unzReadCurrentFile(unzipFile, buffer, cBuffer);
        if (cBytesRead < 0) {
            errorString = QStringLiteral("unzReadCurrentFile failed %1").arg(unzError);
            goto Cleanup;
        }

        if (cBytesRead > 0) {
            qint64 cBytesWritten = decompressedFile.write(static_cast<char*>(buffer), cBytesRead);
            if (cBytesWritten != cBytesRead) {
                errorString = QStringLiteral("Decompressed file write failed %1").arg(decompressedFile.errorString());
                goto Cleanup;
            }
        }

    } while (cBytesRead > 0);

Cleanup:
    bool success = unzError == UNZ_OK && !errorString.isEmpty();

    free(buffer);
    if (decompressedFile.isOpen()) {
        decompressedFile.close();
        if (!success) {
            decompressedFile.remove();
        }
    }
    if (unzipFile) {
        unzClose(unzipFile);
    }
    return success ? decompressedFile.fileName() : QString();
}
