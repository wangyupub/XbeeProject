/**
 * 
 */
package com.truthfulgiant.xbeeswitchcontrol.protocol;


/**
 * @author ywang
 *
 */
public class SetSingleSwitchCommand extends AbstractCommand {

	public SetSingleSwitchCommand() {
		super(CommandType.COMMAND_GET_SINGLE_SWITCH);
	}
	
	public SetSingleSwitchCommand(short switchIndex, byte switchStatus) {
		this();
		
		AppendParam(switchIndex);
		AppendParam(switchStatus);
	}
	
	public SetSingleSwitchCommand(com.truthfulgiant.xbeeswitchcontrol.protocol.generated.SetSingleSwitchCommand c) {
		this((short) c.getSwitchIndex(), (byte) c.getSwitchStatus());
	}
}
