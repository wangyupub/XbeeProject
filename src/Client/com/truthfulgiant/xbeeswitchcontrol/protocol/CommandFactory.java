/**
 * 
 */
package com.truthfulgiant.xbeeswitchcontrol.protocol;

/**
 * @author dreamer
 * To be clear, this is not strictly a Factory Method / Abstract Factory pattern.
 */
public class CommandFactory {

	public static AbstractCommand CreateCommand(com.truthfulgiant.xbeeswitchcontrol.protocol.generated.AbstractCommand c) {
		if (c.getClass() == com.truthfulgiant.xbeeswitchcontrol.protocol.generated.SetSingleSwitchCommand.class) {
			return new SetSingleSwitchCommand((com.truthfulgiant.xbeeswitchcontrol.protocol.generated.SetSingleSwitchCommand) c);
		}
		else if (c.getClass() == com.truthfulgiant.xbeeswitchcontrol.protocol.generated.GetSingleSwitchCommand.class) {
			return new GetSingleSwitchCommand((com.truthfulgiant.xbeeswitchcontrol.protocol.generated.GetSingleSwitchCommand) c);
		}
		else if (c.getClass() == com.truthfulgiant.xbeeswitchcontrol.protocol.generated.SetMultipleSwitchesCommand.class) {
			return new SetMultipleSwitchesCommand((com.truthfulgiant.xbeeswitchcontrol.protocol.generated.SetMultipleSwitchesCommand) c);
		}
		else if (c.getClass() == com.truthfulgiant.xbeeswitchcontrol.protocol.generated.GetMultipleSwitchesCommand.class) {
			return new GetMultipleSwitchesCommand((com.truthfulgiant.xbeeswitchcontrol.protocol.generated.GetMultipleSwitchesCommand) c);
		}
		else if (c.getClass() == com.truthfulgiant.xbeeswitchcontrol.protocol.generated.GetNumSwitchesCommand.class) {
			return new GetNumSwitchesCommand((com.truthfulgiant.xbeeswitchcontrol.protocol.generated.GetNumSwitchesCommand) c);
		}
		else if (c.getClass() == com.truthfulgiant.xbeeswitchcontrol.protocol.generated.SetSingleSwitchDelayCommand.class) {
			return new SetSingleSwitchDelayCommand((com.truthfulgiant.xbeeswitchcontrol.protocol.generated.SetSingleSwitchDelayCommand) c);
		}
		else if (c.getClass() == com.truthfulgiant.xbeeswitchcontrol.protocol.generated.PassThroughCommand.class) {
			return new PassThroughCommand((com.truthfulgiant.xbeeswitchcontrol.protocol.generated.PassThroughCommand) c);
		}
		else if (c.getClass() == com.truthfulgiant.xbeeswitchcontrol.protocol.generated.CompositeCommand.class) {
			return new CompositeCommand((com.truthfulgiant.xbeeswitchcontrol.protocol.generated.CompositeCommand) c);
		}
		else {
			return null;
		}
	}
}
