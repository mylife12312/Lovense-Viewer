/** 
 * @file QrToPng.h
 * @brief QrToPng class header file
 *
 * $LicenseInfo:firstyear=2023&license=viewerlgpl$
 * Lovense Viewer Source Code
 * Copyright (C) 2023, HYTTO PTE. LTD.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * $/LicenseInfo$
 */

#ifndef QR_TO_PNG_H
#define QR_TO_PNG_H

/* If your compiler is recent enough,
 * you don't need to include '::experimental::',
 * you can just include "::filesystem". The below
 * code makes both work, accessible at 'fs::'. */
#include <filesystem>
namespace fs = std::filesystem;

#include "QrCode.hpp"
#include "TinyPngOut.hpp"
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

class QrToPng {
public:
    /**
     * Gives an object containing all the data to create the QR code. When @writeToPNG() is called,
     * the actual file is constructed and written.
     * The image is scaled to fit in the given size as much as possible relative to the QR code
     * size.
     * @param fileName relative or absolute filename to write image to. Relative will be in CWD.
     * @param imgSize The height and width of the image. Image is square, so will be width and height.
     * @param minModulePixelSize How many pixels big should a qr module be (a white or black dot)?
     * @param text The text to encode in the QR code.
     * @param overwriteExistingFile Overwrite if a file with @fileName already exists?
     * @param ecc error correction (low,mid,high).
     */
    QrToPng(std::string fileName, int imgSize, int minModulePixelSize, std::string text,
        bool overwriteExistingFile, qrcodegen::QrCode::Ecc ecc);

    /** Writes a QrToPng object to a png file at @_fileName.
     * @return true if file could be written, false if file could not be written */
    bool writeToPNG();

private:
    std::string _fileName;
    int _size;
    int _minModulePixelSize;
    std::string _text;
    bool _overwriteExistingFile;
    qrcodegen::QrCode::Ecc _ecc;

    /** Writes the PNG file. Constructs a vector with
     * each element being a row of RGB 8.8.8 pixels, the
     * format is geared towards the tinypngoutput library.
     * @param qrData the code returned by the qrcodegen library
     * @return true if file could be written, false if file could not be written */
    [[nodiscard]] bool _writeToPNG(const qrcodegen::QrCode& qrData) const;

    /* returns the width/height of the image based on the max image size
    * and qr width. Ex. If the max img size is 90, the qr code size 29
    * the qr module pixel size will be 3, the image size will be 3*29=87. */
    [[nodiscard]] uint32_t _imgSize(const qrcodegen::QrCode& qrData) const;

    [[nodiscard]] uint32_t _imgSizeWithBorder(const qrcodegen::QrCode& qrData) const;
};

#endif //QR_TO_PNG_H
