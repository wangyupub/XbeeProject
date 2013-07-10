/**
 * 
 */
package com.truthfulgiant.xbeeswitchcontrol.protocol;

/**
 * @author ywang
 *
 */
public class GetMultipleSwitchesCommand extends AbstractCommand {
	public GetMultipleSwitchesCommand() {
		super(CommandType.COMMAND_GET_MULTIPLE_SWITCHES);
	}
	
	public GetMultipleSwitchesCommand(short switchOffset) {
		this();
		AppendParam(switchOffset);
	}
	
	public GetMultipleSwitchesCommand(com.truthfulgiant.xbeeswitchcontrol.protocol.generated.GetMultipleSwitchesCommand c) {
		this((short) c.getSwitchOffset());
	}
}
