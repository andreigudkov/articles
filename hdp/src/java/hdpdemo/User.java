package hdpdemo;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import org.apache.hadoop.io.Writable;

public class User implements Writable {
	public long uid;       // user ID
	public byte[] payload; // other data: age, location, gender, preferences

	public User() {
	}

	public User(User from) {
		this.uid = from.uid;
		this.payload = Utils.cloneByteArray(from.payload);
	}

	@Override
	public void write(DataOutput out) throws IOException {
		out.writeLong(uid);
		out.writeInt(payload.length);
		out.write(payload);
	}

	@Override
	public void readFields(DataInput in) throws IOException {
		uid = in.readLong();
		payload = new byte[in.readInt()];
		in.readFully(payload);
	}

}