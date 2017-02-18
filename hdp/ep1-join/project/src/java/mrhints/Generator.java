package mrhints;

import java.util.Random;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.io.SequenceFile;
import org.apache.hadoop.io.SequenceFile.Writer;

public class Generator {

	private static final long SESSIONS_BYTES = 10L*1000*1000*1000;
	private static final int USERS_COUNT = 1000;
	private static final int USER_SIZE = 1024;
	private static final int SESSION_SIZE = 512*1024;

	private Random random = new Random();

	private Session generateSession() {
		Session session = new Session();
		if (random.nextInt(2) == 0) {
			session.uid = 0; // >= 50% for user 0
		} else {
			session.uid = random.nextInt(USERS_COUNT);
		}
		session.data = new byte[SESSION_SIZE];
		random.nextBytes(session.data);
		return session;
	}

	private User generateUser(int uid) {
		User user = new User();
		user.uid = uid;
		user.data = new byte[USER_SIZE];
		random.nextBytes(user.data);
		return user;
	}

	public void generateUsers() throws Exception {
		System.err.println("Writing /users");

		Configuration conf = new Configuration();
		Path path = new Path("/users");
		SequenceFile.Writer writer = SequenceFile.createWriter(
			conf,
			Writer.file(path),
			Writer.keyClass(NullWritable.class),
			Writer.valueClass(Any.class)
		);

		long last = System.currentTimeMillis();
		for (int i = 0; i < USERS_COUNT; i++) {
			User user = generateUser(i);
			writer.append(NullWritable.get(), new Any(user));

			long now = System.currentTimeMillis();
			if (now - last > 1000) {
				System.err.println(String.format("%d/%d", i, USERS_COUNT));
				last = now;
			}
		}
		writer.close();
	}



	public void generateSessions() throws Exception {
		System.err.println("Writing /sessions");

		Configuration conf = new Configuration();
		Path path = new Path("/sessions");
		SequenceFile.Writer writer = SequenceFile.createWriter(
			conf,
			Writer.file(path),
			Writer.keyClass(NullWritable.class),
			Writer.valueClass(Any.class)
		);

		long left = SESSIONS_BYTES;
		long last = System.currentTimeMillis();
		while (left > 0) {
			Session session = generateSession();
			writer.append(NullWritable.get(), new Any(session));
			left -= session.data.length;

			long now = System.currentTimeMillis();
			if (now - last > 1000) {
				System.err.println(left);
				last = now;
			}
		}
		writer.close();
	}

	public void run() throws Exception {
		generateUsers();
		generateSessions();
	}

	public static void main(String[] args) throws Exception {
		new Generator().run();
	}

}
