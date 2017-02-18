package hdpdemo.ep2;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.io.SequenceFile;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.SequenceFileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.SequenceFileOutputFormat;

import hdpdemo.Any;
import hdpdemo.BloomFilter;
import hdpdemo.Session;

public class SelectJobV3 {
  
  private static final String CONF_URL_PATH = "conf.url.path";

	public static class MapperImpl extends Mapper<NullWritable, Any, IntWritable, Any> {
	  
	  private BloomFilter urls = new BloomFilter(32*1024*1024*8, 50); // 32 MiB with 50 hash functions

		@Override
    protected void setup(Mapper<NullWritable, Any, IntWritable, Any>.Context context)
        throws IOException, InterruptedException {
		  
      super.setup(context);
		  
		  Configuration conf = context.getConfiguration();
		  Path path = new Path(conf.get(CONF_URL_PATH));
		  SequenceFile.Reader reader = new SequenceFile.Reader(conf, SequenceFile.Reader.file(path));
		  NullWritable key = NullWritable.get();
		  Any value = new Any();
		  while (reader.next(key, value)) {
		    urls.add(value.url.getBytes());
		    context.getCounter(InputRecordByType.Url).increment(1);
		  }
		  reader.close();
    }

    @Override
		protected void map(NullWritable key, Any value, Mapper<NullWritable, Any, IntWritable, Any>.Context context)
			throws IOException, InterruptedException {

			if (value.session != null) {
        context.getCounter(InputRecordByType.Session).increment(1);
			  if (urls.contains(value.session.url.getBytes())) {
          context.write(new IntWritable(value.session.url.hashCode()), new Any(value.session));
			  } else {
			    context.getCounter(SelectStatus.SessionSkipped).increment(1);
			  }
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
		job.getConfiguration().set(CONF_URL_PATH, "/urls");
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
		new SelectJobV3().run();
	}
}
