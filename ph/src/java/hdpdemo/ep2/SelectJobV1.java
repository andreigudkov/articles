package hdpdemo.ep2;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

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

public class SelectJobV1 {
  
  private static final String CONF_URL_PATH = "conf.url.path";

	public static class MapperImpl extends Mapper<NullWritable, Any, IntWritable, Any> {
	  
	  private List<String> urls = new ArrayList<>();

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
		    urls.add(value.url);
		  }
		  reader.close();
		  
		  Collections.sort(urls);
			context.getCounter(InputRecordByType.Url).increment(urls.size());
    }

    @Override
		protected void map(NullWritable key, Any value, Mapper<NullWritable, Any, IntWritable, Any>.Context context)
			throws IOException, InterruptedException {

			if (value.session != null) {
				context.getCounter(InputRecordByType.Session).increment(1);
				if (Collections.binarySearch(urls, value.session.url) >= 0) {
          context.write(new IntWritable(value.session.url.hashCode()), new Any(value.session));
          context.getCounter(SelectStatus.SessionSelected).increment(1);
				} else {
				  context.getCounter(SelectStatus.SessionSkipped).increment(1);
				}
			}
		}
	}

	public static class ReducerImpl extends Reducer<IntWritable, Any, NullWritable, Any> {

		@Override
		protected void reduce(IntWritable key, Iterable<Any> values, Reducer<IntWritable, Any, NullWritable, Any>.Context context)
				throws IOException, InterruptedException {
		  
		  for (Any value : values) {
		    context.write(NullWritable.get(), value);
		  }
		}
	}

	public void run() throws Exception {
		Job job = Job.getInstance();
		job.setJobName(this.getClass().getSimpleName());
		job.setJarByClass(this.getClass());

		job.setInputFormatClass(SequenceFileInputFormat.class);
		SequenceFileInputFormat.addInputPath(job, new Path("/sessions"));
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
		new SelectJobV1().run();
	}
}
