/**
 * 
 */
package com.truthfulgiant.xbeeswitchcontrol.protocol;

import java.util.Vector;

/**
 * @author ywang
 *
 */
public class CompositeCommand extends AbstractCommand {
	
	/**
	 * Default constructor
	 */
	CompositeCommand() {
		super(CommandType.MULTIPLE_COMMAND_PACK);
	}
	
	@Override
	public int AddCommand(AbstractCommand c) {
		// composite command cannot be added again
		if (c.GetCommandType() != CommandType.MULTIPLE_COMMAND_PACK) {
			commands.add(c);
		}
		else {
			//TODO: throw Exception
		}
		return commands.size();
	}
	
	@Override
	public AbstractCommand GetCommand(int i) {
		return commands.get(i);
	}
	
	@Override
	public void BuildByteBuffer() {
		buffer.clear();
		
		buffer.put(GetCommandType());
		buffer.put((byte) commands.size());		
		for (AbstractCommand c : commands) {
			c.BuildByteBuffer();
			buffer.put(c.GetByteArray());
		}
	}
	
	Vector<AbstractCommand> commands;
}
