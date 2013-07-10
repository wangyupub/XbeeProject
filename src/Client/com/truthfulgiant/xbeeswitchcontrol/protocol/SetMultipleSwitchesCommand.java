/**
 * 
 */
package com.truthfulgiant.xbeeswitchcontrol.protocol;

/**
 * @author ywang
 *
 */
public class SetMultipleSwitchesCommand extends AbstractCommand {
	public SetMultipleSwitchesCommand() {
		super(CommandType.COMMAND_SET_MULTIPLE_SWITCHES);
	}
	
	public SetMultipleSwitchesCommand(short switchOffset, long switchBitMask) {
		this();
		AppendParam(switchOffset);
		AppendParam(switchBitMask);
	}
	
	public SetMultipleSwitchesCommand(com.truthfulgiant.xbeeswitchcontrol.protocol.generated.SetMultipleSwitchesCommand c) {
		this((short) c.getSwitchOffset(), c.getSwitchBitMask().longValue());
	}
}
