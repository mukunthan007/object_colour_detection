// Generated by `package:ffigen`.
// ignore_for_file: type=lint
import 'dart:ffi' as ffi;
import 'dart:io';
import "package:ffi/ffi.dart";

/// Bindings to opencv and zxing.
class GeneratedBindings {
  /// Holds the symbol lookup function.
  final ffi.Pointer<T> Function<T extends ffi.NativeType>(String symbolName)
      _lookup;

  /// The symbols are looked up in [dynamicLibrary].
  GeneratedBindings(ffi.DynamicLibrary dynamicLibrary)
      : _lookup = dynamicLibrary.lookup;

  /// The symbols are looked up with [lookup].
  GeneratedBindings.fromLookup(
      ffi.Pointer<T> Function<T extends ffi.NativeType>(String symbolName)
          lookup)
      : _lookup = lookup;

  /// @brief Processes image bytes.
  /// @param bytes Image bytes.
  /// @param width Image width.
  /// @param height Image height.
  /// @return Image bytes.
  ///

  /*final _getString =
      conert_c_string.asFunction<String Function(ffi.Pointer<ffi.Uint8>)>();
  final conert_c_string () {
    
  }*/

  String opencvProcessStream(ffi.Pointer<ffi.Char> bytes, int width, int height,
      ffi.Pointer<ffi.Char> kitType) {
    print("+++++++++++++ bytes $bytes");

    final test = _opencvProcessStream(bytes, width, height, kitType);

    String temp = test.toDartString();
    print("test $test");
    print("tes_string $temp");

    return temp;
  }

  late final _opencvProcessStreamPtr = _lookup<
      ffi.NativeFunction<
          ffi.Pointer<Utf8> Function(ffi.Pointer<ffi.Char>, ffi.Int, ffi.Int,
              ffi.Pointer<ffi.Char>)>>('opencvProcessStream');
  late final _opencvProcessStream = _opencvProcessStreamPtr.asFunction<
      ffi.Pointer<Utf8> Function(
          ffi.Pointer<ffi.Char>, int, int, ffi.Pointer<ffi.Char>)>();

  void opencvProcessImage(
    ffi.Pointer<ffi.Char> input,
    ffi.Pointer<ffi.Char> output,
  ) {
    return _opencvProcessImage(
      input,
      output,
    );
  }

  late final _opencvProcessImagePtr = _lookup<
      ffi.NativeFunction<
          ffi.Void Function(ffi.Pointer<ffi.Char>,
              ffi.Pointer<ffi.Char>)>>('opencvProcessImage');
  late final _opencvProcessImage = _opencvProcessImagePtr.asFunction<
      void Function(ffi.Pointer<ffi.Char>, ffi.Pointer<ffi.Char>)>();

  /// Returns the version of the zxing library.
}

abstract class Format {
  /// < Used as a return value if no valid barcode has been detected
  static const int None = 0;

  /// < Aztec (2D)
  static const int Aztec = 1;

  /// < Codabar (1D)
  static const int Codabar = 2;

  /// < Code39 (1D)
  static const int Code39 = 4;

  /// < Code93 (1D)
  static const int Code93 = 8;

  /// < Code128 (1D)
  static const int Code128 = 16;

  /// < GS1 DataBar, formerly known as RSS 14
  static const int DataBar = 32;

  /// < GS1 DataBar Expanded, formerly known as RSS EXPANDED
  static const int DataBarExpanded = 64;

  /// < DataMatrix (2D)
  static const int DataMatrix = 128;

  /// < EAN-8 (1D)
  static const int EAN8 = 256;

  /// < EAN-13 (1D)
  static const int EAN13 = 512;

  /// < ITF (Interleaved Two of Five) (1D)
  static const int ITF = 1024;

  /// < MaxiCode (2D)
  static const int MaxiCode = 2048;

  /// < PDF417 (1D) or (2D)
  static const int PDF417 = 4096;

  /// < QR Code (2D)
  static const int QRCode = 8192;

  /// < UPC-A (1D)
  static const int UPCA = 16384;

  /// < UPC-E (1D)
  static const int UPCE = 32768;
  static const int OneDCodes = 51070;
  static const int TwoDCodes = 14465;
  static const int Any = 65535;
}

final class CodeResult extends ffi.Struct {
  @ffi.Int()
  external int isValid;

  external ffi.Pointer<ffi.Char> text;

  @ffi.Int32()
  external int format;
}

final class EncodeResult extends ffi.Struct {
  external ffi.Pointer<ffi.UnsignedInt> data;

  @ffi.Int()
  external int length;

  @ffi.Int()
  external int isValid;

  external ffi.Pointer<ffi.Char> error;
}
