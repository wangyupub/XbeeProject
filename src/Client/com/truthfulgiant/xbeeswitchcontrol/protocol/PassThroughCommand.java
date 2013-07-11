/**
 * 
 */
package com.truthfulgiant.xbeeswitchcontrol.protocol;

/**
 * @author ywang
 *
 */
public class PassThroughCommand extends AbstractCommand {
	public PassThroughCommand() {
		super(CommandType.COMMAND_PASS_THROUGH);
	}
	
	public PassThroughCommand(byte[] data) {
		this();
		AppendParam(data);
	}
	
	public PassThroughCommand(String data) {
		this();
		//treating string as single byte
		AppendParam((byte) data.length());
		for (char c : data.toCharArray()) {
			AppendParam((byte) c);
		}
	}
	
	public PassThroughCommand(com.truthfulgiant.xbeeswitchcontrol.protocol.generated.PassThroughCommand c) {
		this(c.getData());
	}
	
	protected void AppendParam(byte[] byteArray) {
		AppendParam((byte) byteArray.length);
		for (byte b : byteArray) {
			AppendParam(b);
		}
	}
}
