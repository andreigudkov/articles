package mrhints;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import org.apache.hadoop.io.Writable;

public class Any implements Writable {
	public Session session;
	public User user;

	public Any() {
	}

	public Any(Session session, User user) {
		this.session = session;
		this.user = user;
	}

	public Any(Session session) {
		this(session, null);
	}

	public Any(User user) {
		this(null, user);
	}

	@Override
	public void write(DataOutput out) throws IOException {
		writeField(out, session);
		writeField(out, user);
	}

	@Override
	public void readFields(DataInput in) throws IOException {
		session = readField(in, new Session());
		user = readField(in, new User());
	}

	private void writeField(DataOutput out, Writable writable) throws IOException {
		if (writable != null) {
			out.writeBoolean(true);
			writable.write(out);
		} else {
			out.writeBoolean(false);
		}
	}

	private <T extends Writable> T readField(DataInput in, T t) throws IOException {
		if (in.readBoolean()) {
			t.readFields(in);
			return t;
		} else {
			return null;
		}
	}
}
