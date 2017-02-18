package hdpdemo;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import org.apache.hadoop.io.Writable;

public class Any implements Writable {
  private static final int FIELD_SESSION = 0x1;
  private static final int FIELD_USER = 0x2;
  private static final int FIELD_URL = 0x4;

	public Session session;
	public User user;
	public String url;

	public Any() {
	}
	
	public Any(Session session, User user) {
	  this.session = session;
	  this.user = user;
	}

	public Any(Session session) {
	  this.session = session;
	}

	public Any(User user) {
	  this.user = user;
	}
	
	public Any(String url) {
	  this.url = url;
	}

	@Override
	public void write(DataOutput out) throws IOException {
	  int mask = 0;
	  if (session != null) {
	    mask |= FIELD_SESSION;
	  }
	  if (user != null) {
	    mask |= FIELD_USER;
	  }
	  if (url != null) {
	    mask |= FIELD_URL;
	  }
	  out.writeByte(mask);
	  
	  if (session != null) {
	    session.write(out);
	  }
	  if (user != null) {
	    user.write(out);
	  }
	  if (url != null) {
	    out.writeUTF(url);
	  }
	}

	@Override
	public void readFields(DataInput in) throws IOException {
	  int mask = in.readByte();

	  if ((mask & FIELD_SESSION) != 0) {
	    if (session == null) {
	      session = new Session();
	    }
	    session.readFields(in);
	  } else {
	    session = null;
	  }
	  
	  if ((mask & FIELD_USER) != 0) {
	    if (user == null) {
	      user = new User();
	    }
	    user.readFields(in);
	  } else {
	    user = null;
	  }
	  
	  if ((mask & FIELD_URL) != 0) {
	    url = in.readUTF();
	  }
	}
}
