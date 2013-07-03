/**
 * 
 */
package com.truthfulgiant.xbeeswitchcontrol.protocol;

import java.io.IOException;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.util.Vector;

class CommandType {
	public static final byte COMMAND_INVALID = (byte) 0xEF;
	
	public static final byte COMMAND_SET_SINGLE_SWITCH = (byte) 0xC1;
	public static final byte COMMAND_GET_SINGLE_SWITCH = (byte) 0xC2;
	public static final byte COMMAND_SET_MULTIPLE_SWITCH = (byte) 0xC3;
	public static final byte COMMAND_GET_MULTIPLE_SWITCH = (byte) 0xC4;
	public static final byte COMMAND_GET_NUM_SWITCHES = (byte) 0xC5;
	public static final byte COMMAND_SET_SINGLE_SWITCH_DELAY = (byte) 0xC6;
	public static final byte COMMAND_PASS_THROUGH = (byte) 0xC7;
	
	public static final byte COMMAND_INSTANT_MASK = (byte) 0xC0;

	public static final byte COMMAND_SET_SCHEDULED_TASK = (byte) 0x31;
	public static final byte COMMAND_GET_GET_SCHEDULED_TASK = (byte) 0x32;
	public static final byte COMMAND_DELETE_SCHEDULED_TASK = (byte) 0x33;

	public static final byte COMMAND_SCHEDULED_MASK = (byte) 0x30;
	
	public static final byte MULTIPLE_COMMAND_PACK = (byte) 0xAC;
}

/**
 * @author ywang
 *
 */
public abstract class AbstractCommand {
	
	AbstractCommand(Byte type) {
		this.commandType = type;
		parameters.clear();
	}
	
	/**
	 * Builds byte buffer for network transmission.
	 */
	public void BuildByteBuffer() {
		buffer.clear();

		// put command type first
		buffer.put(GetCommandType());
		
		for (Number n : parameters) {
			if (n instanceof Byte) {
				buffer.put(n.byteValue());
			}
			else if (n instanceof Short) {
				buffer.putShort(n.shortValue());
			}
			else if (n instanceof Integer) {
				buffer.putInt(n.intValue());
			}
			else if (n instanceof Long) {
				buffer.putLong(n.longValue());
			}
			else {
//TODO:				throw Exception;
			}
		}
	}
	
	/**
	 * Gets the command as byte array.
	 * @return The byte array ready to be send to a socket.
	 */
	protected byte[] GetByteArray() {
		return buffer.array();
	}
	
	/**
	 * Appends parameter to the parameter list
	 * @param param Parameter to be added to the end of the parameter list.
	 * @return The index of the parameter added.
	 */
	public int AppendParam(Number param) {
		parameters.add(param);
		return (parameters.size() - 1);
	}
	
	/**
	 * Interface to add command which does nothing for non composite command.
	 * @param c Command to be added.
	 * @return Number of commands within the composite command after the addition.
	 */
	public int AddCommand(AbstractCommand c) {
		//TODO: throw Exception
		return 0;
	}
	
	/**
	 * Returns the ith Command within the composite command.
	 * @param i The index of the command to be returned.
	 * @return Command with the index specified in the argument, null if not existed.
	 */
	public AbstractCommand GetCommand(int i) {
		return null;
	}
	
	/**
	 * Returns command type.
	 * @return Command type as a byte value.
	 */
	public byte GetCommandType() {
		return commandType.byteValue();
	}
	
	/**
	 * Sends the command through a socket.
	 * @param socket A connected socket to send the command.
	 * @throws IOException
	 */
	public void Send(Socket socket) throws IOException {
		BuildByteBuffer();
		socket.getOutputStream().write(GetByteArray());
	}
	
	protected ByteBuffer buffer;
	protected Vector<Number> parameters;
	protected Byte commandType;
	// this is reserved for further return data manipulation
	protected Integer serialNo;
}
