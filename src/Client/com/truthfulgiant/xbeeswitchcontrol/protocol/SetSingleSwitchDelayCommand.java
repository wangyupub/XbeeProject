/**
 * 
 */
package com.truthfulgiant.xbeeswitchcontrol.protocol;

/**
 * @author ywang
 *
 */
public class SetSingleSwitchDelayCommand extends AbstractCommand {
	public SetSingleSwitchDelayCommand() {
		super(CommandType.COMMAND_SET_SINGLE_SWITCH_DELAY);
	}
	
	public SetSingleSwitchDelayCommand(short switchIndex, byte switchStatus, short delay) {
		this();
		AppendParam(switchIndex);
		AppendParam(switchStatus);
		AppendParam(delay);
	}
	
	public SetSingleSwitchDelayCommand(com.truthfulgiant.xbeeswitchcontrol.protocol.generated.SetSingleSwitchDelayCommand c) {
		this((short) c.getSwitchIndex(), (byte) c.getSwitchStatus(), (short) c.getDelay());
	}
}
