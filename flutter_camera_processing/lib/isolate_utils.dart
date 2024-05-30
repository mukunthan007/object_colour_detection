import 'dart:isolate';
import 'dart:math';
import 'dart:typed_data';
import 'dart:io';
import 'dart:ui';

import 'package:camera/camera.dart';

import 'flutter_camera_processing.dart';
import 'image_converter.dart';
import 'package:image/image.dart' as imglib;
import 'package:google_mlkit_text_recognition/google_mlkit_text_recognition.dart';

// Inspired from https://github.com/am15h/object_detection_flutter

/// Bundles data to pass between Isolate

class OpenCVIsolateData {
  CameraImage cameraImage;
  InputImage? inputImage;
  String kitType;
  SendPort? responsePort;

  OpenCVIsolateData(this.cameraImage, this.inputImage, this.kitType);
}

/// Manages separate Isolate instance for inference
class IsolateUtils {
  static const String kDebugName = "IsolateProcessing";

  Isolate? _isolate;
  final _receivePort = ReceivePort();
  SendPort? _sendPort;

  SendPort? get sendPort => _sendPort;

  Future<void> startIsolateProcessing() async {
    _isolate = await Isolate.spawn<SendPort>(
      processingEntryPoint,
      _receivePort.sendPort,
      debugName: kDebugName,
    );

    _sendPort = await _receivePort.first;
  }

  void stopIsolateProcessing() {
    _isolate?.kill(priority: Isolate.immediate);
    _isolate = null;
    _sendPort = null;
  }

  static void processingEntryPoint(SendPort sendPort) async {
    final port = ReceivePort();
    sendPort.send(port.sendPort);

    await for (final isolateData in port) {
      if (isolateData is OpenCVIsolateData) {
        final inputImage = isolateData.inputImage;
        final image = isolateData.cameraImage;
        final kit_type = isolateData.kitType;
        final bytes = await convertImage(inputImage, image);
        print("before byte $bytes");

        final jsonDatas = FlutterCameraProcessing.opencvProcessStream(
            bytes, image.width, image.height, kit_type);
        print("after func $jsonDatas");
        //var width_1 = image.width;
        //print("+++++++++++++ finished bytes $width_1 ,$image.height");
        //final result = Uint8List.fromList(jsonDatas['result']);
        //print("result $result");
        /*final img = imglib.Image.fromBytes(
          width: image.width,
          height: image.height,
          bytes: result.buffer,
          numChannels: 3,
        );
        final resultBytes = Uint32List.fromList(imglib.encodeJpg(img));*/
        isolateData.responsePort?.send(jsonDatas);
      }
    }
  }
}
