package com.truthfulgiant.xbeeswitchcontrol.protocol;

public class GetNumSwitchesCommand extends AbstractCommand {
	public GetNumSwitchesCommand() {
		super(CommandType.COMMAND_GET_NUM_SWITCHES);
	}
	
	public GetNumSwitchesCommand(com.truthfulgiant.xbeeswitchcontrol.protocol.generated.GetNumSwitchesCommand c) {
		this();
	}
}
