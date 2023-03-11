import 'dart:convert';
import 'dart:typed_data';

import 'package:flutter/material.dart';
import 'package:flutter_bluetooth_serial/flutter_bluetooth_serial.dart';

void main() {
  runApp(new MaterialApp(
    home: new HalamanSatu(),
  ));
}

class HalamanSatu extends  StatelessWidget{
  @override
  Widget build(BuildContext context){
    return new Scaffold(
      body: new Container(
        child: TextButton(
          onPressed: () {

          },
          child: Text(
            "Let's take a picture",
          ),
        ),
      ),
    )
  }
  Future<void> _testKirim() async {
    try {
      BluetoothConnection connection = await BluetoothConnection.toAddress(widget.server.address);
      print('Connected to the device');

      connection.input.listen((Uint8List data) {
        print('Data incoming: ${ascii.decode(data)}');
        connection.output.add(data); // Sending data

        if (ascii.decode(data).contains('!')) {
          connection.finish(); // Closing connection
          print('Disconnecting by local host');
        }
      }).onDone(() {
        print('Disconnected by remote request');
      });
    }
    catch (exception) {
      print('Cannot connect, exception occured');
    }
  }
}

