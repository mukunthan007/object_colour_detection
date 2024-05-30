import 'dart:async';
import 'dart:ffi';
import 'dart:io';
import 'dart:isolate';
import 'dart:typed_data';
import 'dart:convert';

import 'package:camera/camera.dart';
import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';
import 'package:google_mlkit_text_recognition/google_mlkit_text_recognition.dart';

import 'generated_bindings.dart';
import 'isolate_utils.dart';

class FlutterCameraProcessing {
  static const MethodChannel _channel =
      MethodChannel('flutter_camera_processing');

  static Future<String?> get platformVersion async {
    final String? version = await _channel.invokeMethod('getPlatformVersion');
    return version;
  }

  static final bindings = GeneratedBindings(dylib);

  static IsolateUtils? isolateUtils;

  static Future<void> startCameraProcessing() async {
    isolateUtils = IsolateUtils();
    await isolateUtils?.startIsolateProcessing();
  }

  static void stopCameraProcessing() => isolateUtils?.stopIsolateProcessing();

  static Future<String?> opencvProcessCameraImage(
          CameraImage image, InputImage? inputImage, String kitType) async =>
      await _inference(OpenCVIsolateData(image, inputImage, kitType));

  static opencvProcessStream(
      Uint8List bytes, int width, int height, String kitType) {
    String resultBytes = bindings.opencvProcessStream(
      bytes.allocatePointer(),
      width,
      height,
      kitType.toNativeUtf8().cast<Char>(),
    );
    //String jsonString = String.fromCharCodes(resultBytes);
    print("test_string $resultBytes");
    // Parse the String as JSON
    //Map<String, dynamic> jsonData = json.decode(resultBytes);
    /*bool isPink = jsonData.containsKey('color_data');
    if (isPink) {
      print("json_valid");
    }*/
    //print("est_json $jsonData");
    return resultBytes;
  }

  static opencvProcessImage(String input, String output) =>
      bindings.opencvProcessImage(
        input.toNativeUtf8().cast<Char>(),
        output.toNativeUtf8().cast<Char>(),
      );

  /// Runs inference in another isolate
  static Future<dynamic> _inference(dynamic isolateData) async {
    final ReceivePort responsePort = ReceivePort();
    isolateUtils?.sendPort
        ?.send(isolateData..responsePort = responsePort.sendPort);
    final dynamic results = await responsePort.first;
    return results;
  }
}

// Getting a library that holds needed symbols
DynamicLibrary _openDynamicLibrary() {
  if (Platform.isAndroid) {
    return DynamicLibrary.open('libflutter_camera_processing.so');
  } else if (Platform.isWindows) {
    return DynamicLibrary.open("flutter_camera_processing_windows_plugin.dll");
  }
  return DynamicLibrary.process();
}

DynamicLibrary dylib = _openDynamicLibrary();

extension Uint8ListBlobConversion on Uint8List {
  /// Allocates a pointer filled with the Uint8List data.
  Pointer<Char> allocatePointer() {
    final Pointer<Int8> blob = calloc<Int8>(length);
    final Int8List blobBytes = blob.asTypedList(length);
    blobBytes.setAll(0, this);
    return blob.cast<Char>();
  }
}
