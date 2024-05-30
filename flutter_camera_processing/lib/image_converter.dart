import 'dart:io';
import 'package:camera/camera.dart';
import 'package:flutter/foundation.dart';
import 'package:google_mlkit_text_recognition/google_mlkit_text_recognition.dart';
import 'package:image/image.dart' as imglib;
// https://gist.github.com/Alby-o/fe87e35bc21d534c8220aed7df028e03

imglib.Image decodeYUV420SP(InputImage image) {
  final width = image.metadata!.size.width.toInt();
  final height = image.metadata!.size.height.toInt();

  Uint8List yuv420sp = image.bytes!;
  int total = width * height;
  Uint8List rgb = Uint8List(total);
  final outImg =
      imglib.Image(width: width, height: height); // default numChannels is 3
  final int frameSize = width * height;

  for (int j = 0, yp = 0; j < height; j++) {
    int uvp = frameSize + (j >> 1) * width, u = 0, v = 0;
    for (int i = 0; i < width; i++, yp++) {
      int y = (0xff & yuv420sp[yp]) - 16;
      if (y < 0) y = 0;
      if ((i & 1) == 0) {
        v = (0xff & yuv420sp[uvp++]) - 128;
        u = (0xff & yuv420sp[uvp++]) - 128;
      }
      int y1192 = 1192 * y;
      int r = (y1192 + 1634 * v);
      int g = (y1192 - 833 * v - 400 * u);
      int b = (y1192 + 2066 * u);

      if (r < 0) {
        r = 0;
      } else if (r > 262143) {
        r = 262143;
      }
      if (g < 0) {
        g = 0;
      } else if (g > 262143) {
        g = 262143;
      }
      if (b < 0) {
        b = 0;
      } else if (b > 262143) {
        b = 262143;
      }
      outImg.setPixelRgb(i, j, ((r << 6) & 0xff0000) >> 16,
          ((g >> 2) & 0xff00) >> 8, (b >> 10) & 0xff);

      /*rgb[yp] = 0xff000000 |
          ((r << 6) & 0xff0000) |
          ((g >> 2) & 0xff00) |
          ((b >> 10) & 0xff);*/
    }
  }

  return outImg;
}

Future<Uint8List> convertImage(
    InputImage? image, CameraImage cameraImage) async {
  if (image != null) {
    try {
      //final WriteBuffer allBytes = WriteBuffer();
      //for (final Plane plane in image.planes) {
      //allBytes.putUint8List(plane.bytes);
      //}
      //final Uint8List bytes = allBytes.done().buffer.asUint8List();
      /*print("IN normal");
    var temp = convertYUV420ToImage(image);
    final Uint8List imageBytes = temp.getBytes();
    return imageBytes;*/
      /*if (image.format.group == ImageFormatGroup.yuv420) {
      print("IN  yuv");
      var temp = convertYUV420(image);
      final Uint8List imageBytes = temp.getBytes();
      return imageBytes;
    } else if (image.format.group == ImageFormatGroup.bgra8888) {
      print("IN bgra");
      //   // return image.planes.first.bytes;
      return convertBGRA8888(image).getBytes(order: imglib.ChannelOrder.bgra);
    } else {
      final Uint8List bytes = allBytes.done().buffer.asUint8List();
      print("IN normal");
      return bytes;*/
      final Uint8List imageBytes;
      if (cameraImage.format.group == ImageFormatGroup.yuv420) {
        var temp = convertYUV420ToImage(cameraImage);
        imageBytes = temp.getBytes();
      } else if (cameraImage.format.group == ImageFormatGroup.bgra8888) {
        //return image.planes.first.bytes;
        var appleImage = convertBGRA8888(cameraImage);
        imageBytes = appleImage.getBytes();
      } else if (cameraImage.format.group == ImageFormatGroup.nv21) {
        var imageData = decodeYUV420SP(image);
        imageBytes = imageData.getBytes();
      } else {
        return Uint8List(0);
      }
      return imageBytes;
    } catch (e) {
      debugPrint(">>>>>>>>>>>> ERROR:$e");
    }
    return Uint8List(0);
  } else {
    return Uint8List(0);
  }
}

// IOS support
imglib.Image convertBGRA8888(CameraImage image) {
  final plane = image.planes[0];
  return imglib.Image.fromBytes(
    width: image.width,
    height: image.height,
    bytes: plane.bytes.buffer,
    rowStride: plane.bytesPerRow,
    bytesOffset: 28,
    order: imglib.ChannelOrder.bgra,
  );
}

///
/// Converts a [CameraImage] in YUV420 format to [image_lib.Image] in RGB format
///
imglib.Image convertYUV420ToImage(CameraImage cameraImage) {
  final imageWidth = cameraImage.width;
  final imageHeight = cameraImage.height;
  print("Image width $imageWidth  Image height $imageHeight");
  final yBuffer = cameraImage.planes[0].bytes;
  final uBuffer = cameraImage.planes[1].bytes;
  final vBuffer = cameraImage.planes[2].bytes;

  final int yRowStride = cameraImage.planes[0].bytesPerRow;
  final int yPixelStride = cameraImage.planes[0].bytesPerPixel!;

  final int uvRowStride = cameraImage.planes[1].bytesPerRow;
  final int uvPixelStride = cameraImage.planes[1].bytesPerPixel!;

  final image = imglib.Image(width: imageWidth, height: imageHeight);

  for (int h = 0; h < imageHeight; h++) {
    int uvh = (h / 2).floor();

    for (int w = 0; w < imageWidth; w++) {
      int uvw = (w / 2).floor();

      final yIndex = (h * yRowStride) + (w * yPixelStride);

      // Y plane should have positive values belonging to [0...255]
      final int y = yBuffer[yIndex];

      // U/V Values are subsampled i.e. each pixel in U/V chanel in a
      // YUV_420 image act as chroma value for 4 neighbouring pixels
      final int uvIndex = (uvh * uvRowStride) + (uvw * uvPixelStride);

      // U/V values ideally fall under [-0.5, 0.5] range. To fit them into
      // [0, 255] range they are scaled up and centered to 128.
      // Operation below brings U/V values to [-128, 127].
      final int u = uBuffer[uvIndex];
      final int v = vBuffer[uvIndex];

      // Compute RGB values per formula above.
      int r = (y + v * 1436 / 1024 - 179).round();
      int g = (y - u * 46549 / 131072 + 44 - v * 93604 / 131072 + 91).round();
      int b = (y + u * 1814 / 1024 - 227).round();

      r = r.clamp(0, 255);
      g = g.clamp(0, 255);
      b = b.clamp(0, 255);

      image.setPixelRgb(w, h, r, g, b);
    }
  }

  return image;
}
