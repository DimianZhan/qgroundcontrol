/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QString>

/// Unzips a single file from a zip file to a temporary location. It is the callers responsibility
/// to delete the unzipped file after use.
///     @param zipFilename - Full path to zip file to unzip
///     @param[out] errorString -  Error string if error occurs
/// @returns Path to unzipped file. Empty string if error occurs.
/// Not supported:
///     Multiple file archive
///     File names in zip file with a path
QString QGCUnzipSingleFile(const QString& zipFilename, QString& errorString);
