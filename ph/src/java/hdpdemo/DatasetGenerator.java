package hdpdemo;

import java.util.Random;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.io.SequenceFile;
import org.apache.hadoop.io.SequenceFile.Writer;

public class DatasetGenerator {

	private static final int SESSION_COUNT = 20000;
	private static final int USERS_COUNT = 1000;
	private static final int USER_PAYLOAD_SIZE = 1024;
	private static final int SESSION_PAYLOAD_SIZE = 512*1024;

	private Random random = new Random();

	private Session newSession() {
		Session session = new Session();
		if (random.nextInt(2) == 0) {
			session.uid = 0; // >= 50% for user 0
		} else {
			session.uid = random.nextInt(USERS_COUNT);
		}
		session.payload = new byte[SESSION_PAYLOAD_SIZE];
		session.url = newUrl(random.nextInt(SESSION_COUNT));
		random.nextBytes(session.payload);
		return session;
	}
	
	/**
	 * Generate random url deterministically, will return identical urls for identical seeds.
	 */
	private String newUrl(int seed) {
	  Random rnd = new Random(seed);
	  char chars[] = new char[100];
	  for (int i = 0; i < chars.length; i++) {
	    chars[i] = (char) ('a' + rnd.nextInt(26));
	  }
	  return new String(chars);
	}

	private User newUser(int uid) {
		User user = new User();
		user.uid = uid;
		user.payload = new byte[USER_PAYLOAD_SIZE];
		random.nextBytes(user.payload);
		return user;
	}

	public void writeUsers(Path path) throws Exception {
		System.err.println("Writing " + path);

		Configuration conf = new Configuration();
		SequenceFile.Writer writer = SequenceFile.createWriter(
			conf,
			Writer.file(path),
			Writer.keyClass(NullWritable.class),
			Writer.valueClass(Any.class)
		);

		for (int i = 0; i < USERS_COUNT; i++) {
			User user = newUser(i);
			writer.append(NullWritable.get(), new Any(user));
		}
		writer.close();
	}

	public void writeSessions(Path path) throws Exception {
		System.err.println("Writing " + path);

		Configuration conf = new Configuration();
		SequenceFile.Writer writer = SequenceFile.createWriter(
			conf,
			Writer.file(path),
			Writer.keyClass(NullWritable.class),
			Writer.valueClass(Any.class)
		);

		long last = System.currentTimeMillis();
		for (int i = 0; i < SESSION_COUNT; i++) {
		  Session session = newSession();
			writer.append(NullWritable.get(), new Any(session));

			long now = System.currentTimeMillis();
			if (now - last > 1000) {
				System.err.println(String.format("%d/%d", i, SESSION_COUNT));
				last = now;
			}
		}
		writer.close();
	}
	
	public void writeUrls(Path path) throws Exception {
		System.err.println("Writing " + path);

		Configuration conf = new Configuration();
		SequenceFile.Writer writer = SequenceFile.createWriter(
			conf,
			Writer.file(path),
			Writer.keyClass(NullWritable.class),
			Writer.valueClass(Any.class)
		);

		for (int i = 0; i < SESSION_COUNT; i++) {
		  if (random.nextInt(10) == 0) { // 10%
		    String url = newUrl(i);
        writer.append(NullWritable.get(), new Any(url));
		  }
		}
		writer.close();
	}

	public void run() throws Exception {
		writeUsers(new Path("/users"));
		writeSessions(new Path("/sessions"));
		writeUrls(new Path("/urls"));
	}

	public static void main(String[] args) throws Exception {
		new DatasetGenerator().run();
	}

}
