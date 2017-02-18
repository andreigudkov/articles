package hdpdemo;

import java.util.BitSet;
import java.util.Random;

import ie.ucd.murmur.MurmurHash;

public class BloomFilter {
  private BitSet bits;
  private int[] seeds;
  
  public BloomFilter(int numBits, int numHashes) {
    this.bits = new BitSet(numBits);
    this.seeds = new int[numHashes];
    
    Random rnd = new Random();
    for (int i = 0; i < numHashes; i++) {
      seeds[i] = rnd.nextInt();
    }
  }
  
  public void add(byte[] data) {
    for (int i = 0; i < seeds.length; i++) {
      bits.set(bitIndex(data, seeds[i]), true);
    }
  }
  
  public boolean contains(byte[] data) {
    for (int i = 0; i < seeds.length; i++) {
      if (bits.get(bitIndex(data, seeds[i]))) {
        return true;
      }
    }
    return false;
  }
  
  private int bitIndex(byte[] data, int seed) {
    int hash = MurmurHash.hash32(data, data.length, seed);
    return (hash & 0x7fff_ffff) % bits.size();
  }

}
