package mrhints;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import org.apache.hadoop.io.Writable;

public class Session implements Writable {
	public long uid;
	public byte[] data;

	public Session() {
	}

	public Session(Session from) {
		this.uid = from.uid;
		this.data = Utils.cloneByteArray(from.data);
	}

	@Override
	public void write(DataOutput out) throws IOException {
		out.writeLong(uid);
		out.writeInt(data.length);
		out.write(data);
	}

	@Override
	public void readFields(DataInput in) throws IOException {
		uid = in.readLong();
		data = new byte[in.readInt()];
		in.readFully(data);
	}

}