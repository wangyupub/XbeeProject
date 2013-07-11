/**
 * 
 */
package com.truthfulgiant.xbeeswitchcontrol.protocol;

import java.nio.ByteBuffer;
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
	
	CompositeCommand(com.truthfulgiant.xbeeswitchcontrol.protocol.generated.CompositeCommand c){
		this();
		commands = new Vector<AbstractCommand>(16);
		for (com.truthfulgiant.xbeeswitchcontrol.protocol.generated.AbstractCommand command : c.getCommands().getCommand()) {
			AddCommand(CommandFactory.CreateCommand(command));
		}
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
		
		ByteBuffer tempBuffer=ByteBuffer.allocate(1024);
		int bufferSize = 0;
		// put command type first
		tempBuffer.put(GetCommandType());
		bufferSize +=  Byte.SIZE / Byte.SIZE;

		tempBuffer.put((byte) commands.size());		
		bufferSize +=  Byte.SIZE / Byte.SIZE;

		for (AbstractCommand c : commands) {
			c.BuildByteBuffer();
			tempBuffer.put(c.GetByteArray());
			bufferSize +=  c.GetByteArray().length;
		}
		buffer = ByteBuffer.allocate(bufferSize);
		buffer.put(tempBuffer.array(), 0, bufferSize);
		
		System.out.println("Build Command Byte (" + bufferSize + " bytes):" + buffer.asCharBuffer());
	}
	
	Vector<AbstractCommand> commands;
}
