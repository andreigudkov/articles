package hdpdemo.ep1;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.SequenceFileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.SequenceFileOutputFormat;

import hdpdemo.Any;
import hdpdemo.Session;
import hdpdemo.User;

public class DenormalizeJobV1 {

	public static class MapperImpl extends Mapper<NullWritable, Any, LongWritable, Any> {
		@Override
		protected void map(NullWritable key, Any value, Mapper<NullWritable, Any, LongWritable, Any>.Context context)
			throws IOException, InterruptedException {

			if (value.session != null) {  // comes from sessions file(s)
				context.write(new LongWritable(value.session.uid), new Any(value.session));
				context.getCounter(InputRecordByType.Session).increment(1);
			}
			if (value.user != null) {  // comes from users file(s)
				context.write(new LongWritable(value.user.uid), new Any(value.user));
				context.getCounter(InputRecordByType.User).increment(1);
			}
		}
	}

	public static class ReducerImpl extends Reducer<LongWritable, Any, NullWritable, Any> {

		@Override
		protected void reduce(LongWritable key, Iterable<Any> values, Reducer<LongWritable, Any, NullWritable, Any>.Context context)
				throws IOException, InterruptedException {

			List<Session> sessions = new ArrayList<Session>();
			User user = null;
			for (Any value : values) {
				if (value.session != null) {
					sessions.add(new Session(value.session));
				}
				if (value.user != null) {
					if (user == null) {
						user = new User(value.user);
					} else {
						context.getCounter(JoinStatus.DuplicateUser).increment(1);
					}
				}
			}

			if (user == null) {
				context.getCounter(JoinStatus.SessionWithoutUser).increment(sessions.size());
			}
			for (Session session : sessions) {
				context.write(NullWritable.get(), new Any(session, user));
			}
		}
	}

	public void run() throws Exception {
		Job job = Job.getInstance();
		job.setJobName(this.getClass().getSimpleName());
		job.setJarByClass(this.getClass());

		job.setInputFormatClass(SequenceFileInputFormat.class);
		SequenceFileInputFormat.addInputPath(job, new Path("/sessions"));
		SequenceFileInputFormat.addInputPath(job, new Path("/users"));
		job.setMapperClass(MapperImpl.class);
		job.setMapOutputKeyClass(LongWritable.class);
		job.setMapOutputValueClass(Any.class);

		job.setReducerClass(ReducerImpl.class);
		job.setOutputKeyClass(NullWritable.class);
		job.setOutputValueClass(Any.class);
		job.setOutputFormatClass(SequenceFileOutputFormat.class);
		SequenceFileOutputFormat.setOutputPath(job, new Path("/enriched_sessions"));
		job.setNumReduceTasks(10);

		if (!job.waitForCompletion(true)) {
			throw new Exception();
		}
	}

	public static void main(String args[]) throws Exception {
		new DenormalizeJobV1().run();
	}
}
