package com.truthfulgiant.xbeeswitchcontrol.client;

import java.io.*;
import java.net.*;

import com.truthfulgiant.xbeeswitchcontrol.protocol.AbstractCommand;
import com.truthfulgiant.xbeeswitchcontrol.protocol.PassThroughCommand;

public class SwitchControlClient {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		Socket clientSocket = null;
//		PrintWriter out = null;
		BufferedReader in = null;

		try {
			clientSocket = new Socket("127.0.0.1", 3940);
//			out = new PrintWriter(clientSocket.getOutputStream(), true);
			in = new BufferedReader(new InputStreamReader(
					clientSocket.getInputStream()));
		} catch (UnknownHostException e) {
			System.err.println("Don't know about host: localhost.");
			System.exit(1);
		} catch (IOException e) {
			System.err.println("Couldn't get I/O for "
					+ "the connection to: localhost.");
			System.err.println(e.getMessage());
			System.exit(1);
		}

		BufferedReader stdIn = new BufferedReader(new InputStreamReader(
				System.in));
		String userInput;
		try {
			while ((userInput = stdIn.readLine()) != null) {
				System.out.println(userInput + "///");
				AbstractCommand command = new PassThroughCommand(userInput);
				command.Send(clientSocket);
				System.out.println("echo: " + in.read());
			}
		} catch (IOException e) {
			System.err.println("Cannot get user input.");
			System.exit(1);
		}

		try {
//			out.close();
			in.close();
			stdIn.close();
			clientSocket.close();
		} catch (IOException e) {
			System.err.println("Cannot close streams properly.");
			System.exit(1);
		}
	}
}
