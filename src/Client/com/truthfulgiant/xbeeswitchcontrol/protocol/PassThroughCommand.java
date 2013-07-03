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
		AppendParam(data.getBytes());
	}
	
	protected void AppendParam(byte[] byteArray) {
		AppendParam((byte) byteArray.length);
		for (byte b : byteArray) {
			AppendParam(b);
		}
	}
}
