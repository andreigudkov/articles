package hdpdemo.ep2;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.SequenceFileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.SequenceFileOutputFormat;

import hdpdemo.Any;
import hdpdemo.Session;

public class SelectJobV2 {
  
	public static class MapperImpl extends Mapper<NullWritable, Any, IntWritable, Any> {
	  
    @Override
		protected void map(NullWritable key, Any value, Mapper<NullWritable, Any, IntWritable, Any>.Context context)
			throws IOException, InterruptedException {

			if (value.session != null) {
				context.getCounter(InputRecordByType.Session).increment(1);
        context.write(new IntWritable(value.session.url.hashCode()), new Any(value.session));
			}
			if (value.url != null) {
				context.getCounter(InputRecordByType.Url).increment(1);
        context.write(new IntWritable(value.url.hashCode()), new Any(value.url));
			}
		}
	}

	public static class ReducerImpl extends Reducer<IntWritable, Any, NullWritable, Any> {

		@Override
		protected void reduce(IntWritable key, Iterable<Any> values, Reducer<IntWritable, Any, NullWritable, Any>.Context context)
				throws IOException, InterruptedException {
		  
		  Set<String> urls = new HashSet<>();
		  List<Session> sessions = new ArrayList<>();
		  for (Any value : values) {
		    if (value.url != null) {
		      urls.add(value.url);
		    }
		    if (value.session != null) {
		      sessions.add(value.session);
		    }
		  }
		  
		  for (Session session : sessions) {
		    if (urls.contains(session.url)) {
		      context.write(NullWritable.get(), new Any(session));
		      context.getCounter(SelectStatus.SessionSelected).increment(1);
		    } else {
		      context.getCounter(SelectStatus.SessionSkipped).increment(1);
		    }
		  }
		}
	}

	public void run() throws Exception {
		Job job = Job.getInstance();
		job.setJobName(this.getClass().getSimpleName());
		job.setJarByClass(this.getClass());

		job.setInputFormatClass(SequenceFileInputFormat.class);
		SequenceFileInputFormat.addInputPath(job, new Path("/sessions"));
		SequenceFileInputFormat.addInputPath(job, new Path("/urls"));
		job.setMapperClass(MapperImpl.class);
		job.setMapOutputKeyClass(IntWritable.class);
		job.setMapOutputValueClass(Any.class);

		job.setReducerClass(ReducerImpl.class);
		job.setOutputKeyClass(NullWritable.class);
		job.setOutputValueClass(Any.class);
		job.setOutputFormatClass(SequenceFileOutputFormat.class);
		SequenceFileOutputFormat.setOutputPath(job, new Path("/selected_sessions"));
		job.setNumReduceTasks(10);

		if (!job.waitForCompletion(true)) {
			throw new Exception();
		}
	}

	public static void main(String args[]) throws Exception {
		new SelectJobV2().run();
	}
}
