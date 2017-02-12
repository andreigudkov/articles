package mrhints;

import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.IOException;

import org.apache.hadoop.io.Writable;

public class Utils {

	public static int compareLongs(long v1, long v2) {
		if (v1 < v2) {
			return -1;
		} else if (v1 > v2) {
			return 1;
		} else {
			return 0;
		}
	}

	public static int compareInts(int v1, int v2) {
		if (v1 < v2) {
			return -1;
		} else if (v1 > v2) {
			return 1;
		} else {
			return 0;
		}
	}

	public static int compareMultilevel(int... compareResults) {
		for (int r : compareResults) {
			if (r != 0) {
				return r;
			}
		}
		return 0;
	}

	public static byte[] cloneByteArray(byte[] from) {
		byte[] to = new byte[from.length];
		System.arraycopy(from, 0, to, 0, from.length);
		return to;
	}

	public static <T extends Writable> T readObject(T object, byte[] buf, int len, int off) {
	  try {
			object.readFields(new DataInputStream(new ByteArrayInputStream(buf, len, off)));
		} catch (IOException exc) {
			throw new RuntimeException(exc);
		}
	  return object;
	}
}
