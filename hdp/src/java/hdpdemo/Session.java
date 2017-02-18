package hdpdemo;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import org.apache.hadoop.io.Writable;

public class Session implements Writable {
	public long uid;       // user ID
	public String url;     // page URL
	public byte[] payload; // other data: timestamps, events

	public Session() {
	}

	public Session(Session from) {
		this.uid = from.uid;
		this.url = from.url;
		this.payload = Utils.cloneByteArray(from.payload);
	}

	@Override
	public void write(DataOutput out) throws IOException {
		out.writeLong(uid);
		out.writeUTF(url);
		out.writeInt(payload.length);
		out.write(payload);
	}

	@Override
	public void readFields(DataInput in) throws IOException {
		uid = in.readLong();
		url = in.readUTF();
		payload = new byte[in.readInt()];
		in.readFully(payload);
	}

}