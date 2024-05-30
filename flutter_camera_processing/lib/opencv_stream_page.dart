import 'dart:async';
// import 'dart:developer';
import 'dart:io';
import 'dart:math';

import 'package:camera/camera.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_camera_processing/flutter_camera_processing.dart';
import 'package:google_mlkit_text_recognition/google_mlkit_text_recognition.dart';

class OpencvStreamPage extends StatefulWidget {
  const OpencvStreamPage({
    Key? key,
    this.onControllerCreated,
    this.scanFps = const Duration(milliseconds: 100),
    this.resolution = ResolutionPreset.high,
  }) : super(key: key);

  final Function(CameraController?)? onControllerCreated;
  final Duration scanFps;
  final ResolutionPreset resolution;

  @override
  State<OpencvStreamPage> createState() => _OpencvStreamPageState();
}

class _OpencvStreamPageState extends State<OpencvStreamPage> {
  Uint8List? processedImageData;
  late List<CameraDescription> cameras;
  CameraController? controller;
  bool _ml_processing = false;
  bool _status = false;
  bool _isProcessing = false;
  // kit type - tamperLoks, neoKit
  String kitType = "tamperLoks";

  final resultStream = StreamController<Uint8List>.broadcast();

  bool isAndroid() => Theme.of(context).platform == TargetPlatform.android;

  @override
  void initState() {
    super.initState();

    FlutterCameraProcessing.startCameraProcessing();

    availableCameras().then((cameras) {
      setState(() {
        this.cameras = cameras;
        onNewCameraSelected(cameras.first);
      });
    });

    SystemChannels.lifecycle.setMessageHandler((message) async {
      debugPrint(message);
      final CameraController? cameraController = controller;
      if (cameraController == null || !cameraController.value.isInitialized) {
        return;
      }
      if (mounted) {
        if (message == AppLifecycleState.paused.toString()) {
          cameraController.dispose();
        }
        if (message == AppLifecycleState.resumed.toString()) {
          onNewCameraSelected(cameraController.description);
        }
      }
      return null;
    });
  }

  @override
  void dispose() {
    super.dispose();
    FlutterCameraProcessing.stopCameraProcessing();
    controller?.dispose();
  }

  final _orientations = {
    DeviceOrientation.portraitUp: 0,
    DeviceOrientation.landscapeLeft: 90,
    DeviceOrientation.portraitDown: 180,
    DeviceOrientation.landscapeRight: 270,
  };

  InputImage? _inputImageFromCameraImage(CameraImage image,
      CameraController? controller, CameraDescription cameraDescription) {
    // get image rotation
    // it is used in android to convert the InputImage from Dart to Java
    // `rotation` is not used in iOS to convert the InputImage from Dart to Obj-C
    // in both platforms `rotation` and `camera.lensDirection` can be used to compensate `x` and `y` coordinates on a canvas
    final sensorOrientation = cameraDescription.sensorOrientation;
    InputImageRotation? rotation;
    if (Platform.isIOS) {
      rotation = InputImageRotationValue.fromRawValue(sensorOrientation);
    } else if (Platform.isAndroid) {
      var rotationCompensation =
          _orientations[controller!.value.deviceOrientation];
      if (rotationCompensation == null) return null;
      if (cameraDescription.lensDirection == CameraLensDirection.front) {
        // front-facing
        rotationCompensation = (sensorOrientation + rotationCompensation) % 360;
      } else {
        // back-facing
        rotationCompensation =
            (sensorOrientation - rotationCompensation + 360) % 360;
      }
      rotation = InputImageRotationValue.fromRawValue(rotationCompensation);
    }
    if (rotation == null) return null;

    // get image format
    final format = InputImageFormatValue.fromRawValue(image.format.raw);
    // validate format depending on platform
    // only supported formats:
    // * nv21 for Android
    // * bgra8888 for iOS
    if (format == null ||
        (Platform.isAndroid && format != InputImageFormat.nv21) ||
        (Platform.isIOS && format != InputImageFormat.bgra8888)) return null;

    // since format is constraint to nv21 or bgra8888, both only have one plane
    if (image.planes.length != 1) return null;
    final plane = image.planes.first;

    // compose InputImage using bytes
    return InputImage.fromBytes(
      bytes: plane.bytes,
      metadata: InputImageMetadata(
        size: Size(image.width.toDouble(), image.height.toDouble()),
        rotation: rotation, // used only in Android
        format: format, // used only in iOS
        bytesPerRow: plane.bytesPerRow, // used only in iOS
      ),
    );
  }

  void _invalidImageOrNot(InputImage? inputImage) async {
    if (inputImage != null && !_ml_processing) {
      _ml_processing = true;
      final textRecognizer =
          TextRecognizer(script: TextRecognitionScript.latin);
      final RecognizedText recognizedText =
          await textRecognizer.processImage(inputImage);
      textRecognizer.close();
      String sampleString;

      for (TextBlock block in recognizedText.blocks) {
        for (TextLine line in block.lines) {
          // extracting text in line from ml kit
          var textString = line.text;
          var lowerCaseStrig = textString.toLowerCase();
          if (kitType == "tamperLoks") {
            sampleString = "drugs of";
            if (lowerCaseStrig.contains(sampleString)) {
              _status = true;
            }
          } else {
            sampleString = "oral fluid drug test";
            if (lowerCaseStrig == sampleString) {
              _status = true;
            }
          }
        }
      }
      _ml_processing = false;
    }
  }

  void onNewCameraSelected(CameraDescription cameraDescription) async {
    if (controller != null) {
      await controller!.dispose();
    }

    controller = CameraController(
      cameraDescription,
      widget.resolution,
      enableAudio: false,
      imageFormatGroup: Platform.isAndroid
          ? ImageFormatGroup.nv21 // for Android
          : ImageFormatGroup.bgra8888, // for iOS
    );

    try {
      await controller?.initialize();
      controller?.startImageStream((image) async {
        InputImage? inputImage =
            _inputImageFromCameraImage(image, controller, cameraDescription);

        _invalidImageOrNot(inputImage);
        if (_status) {
          processCameraImage(image, inputImage);
          _status = false;
        }
      });
    } on CameraException catch (e) {
      _showCameraException(e);
    }

    controller?.addListener(() {
      if (mounted) setState(() {});
    });

    if (mounted) {
      setState(() {});
    }

    widget.onControllerCreated?.call(controller);
  }

  void _showCameraException(CameraException e) {
    logError(e.code, e.description);
    showInSnackBar('Error: ${e.code}\n${e.description}');
  }

  void showInSnackBar(String message) {}

  void logError(String code, String? message) {
    if (message != null) {
      debugPrint('Error: $code\nError Message: $message');
    } else {
      debugPrint('Error: $code');
    }
  }

  processCameraImage(CameraImage image, InputImage? inputImage) async {
    if (!_isProcessing) {
      _ml_processing = true;
      _isProcessing = true;
      try {
        final result = await FlutterCameraProcessing.opencvProcessCameraImage(
            image, inputImage, kitType);
        print("test $result");
        if (result == null) return;
        /*setState(() {
          processedImageData =
              Uint8List.fromList(result); // Update processedImageData
        });*/
        _isProcessing = false;
      } on FileSystemException catch (e) {
        debugPrint(e.message);
      } catch (e) {
        debugPrint(e.toString());
      }
      _isProcessing = false;
      _ml_processing = false;
    }
    return null;
  }

  @override
  Widget build(BuildContext context) {
    final size = MediaQuery.of(context).size;
    return Scaffold(
      appBar: AppBar(
        title: const Text('Tamper loks'),
      ),
      body: Stack(
        children: [
          // Camera preview
          Center(
              child: _cameraPreviewWidget(
                  processedImageData)), // Pass processedImageData,
        ],
      ),
    );
  }

  Widget _cameraPreviewWidget(Uint8List? processedImageData) {
    final CameraController? cameraController = controller;
    if (cameraController == null || !cameraController.value.isInitialized) {
      return const CircularProgressIndicator();
    } else {
      final size = MediaQuery.of(context).size;
      var cameraMaxSize = max(size.width, size.height);
      return Stack(
        children: [
          SizedBox(
            width: cameraMaxSize,
            height: cameraMaxSize,
            child: ClipRRect(
              child: OverflowBox(
                alignment: Alignment.center,
                child: FittedBox(
                  fit: BoxFit.cover,
                  child: SizedBox(
                    width: cameraMaxSize,
                    child: CameraPreview(
                      cameraController,
                    ),
                  ),
                ),
              ),
            ),
          ),
          if (processedImageData != null)
            Positioned.fill(
              child: RotatedBox(
                quarterTurns: 1,
                child: Image.memory(
                  processedImageData,
                  fit: BoxFit.cover,
                ),
              ),
            ),
        ],
      );
    }
  }
}
