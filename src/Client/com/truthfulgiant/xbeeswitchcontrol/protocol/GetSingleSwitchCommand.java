/**
 * 
 */
package com.truthfulgiant.xbeeswitchcontrol.protocol;

/**
 * @author dreamer
 *
 */
public class GetSingleSwitchCommand extends AbstractCommand {

	/**
	 * @param type
	 */
	public GetSingleSwitchCommand() {
		super(CommandType.COMMAND_GET_SINGLE_SWITCH);
		// TODO Auto-generated constructor stub
	}

	public GetSingleSwitchCommand(short switchIndex) {
		this();
		
		AppendParam(switchIndex);
	}

	public GetSingleSwitchCommand(com.truthfulgiant.xbeeswitchcontrol.protocol.generated.GetSingleSwitchCommand c) {
		this((short) c.getSwitchIndex());
	}
}
