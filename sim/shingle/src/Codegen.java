import java.awt.image.BufferedImage;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.util.ArrayList;
import java.util.List;

import javax.imageio.ImageIO;
import javax.imageio.ImageWriter;
import javax.imageio.stream.ImageOutputStream;

public class Codegen {

  public static void genHeader(String path) throws Exception {
    try (BufferedWriter bw = new BufferedWriter(new FileWriter(path + ".h"))) {
      bw.write("#pragma once\n");
      bw.write("#include <cstdint>\n");
      bw.write("\n");
      bw.write("extern __attribute__((aligned(64))) const uint64_t kAlphanumTable[];\n");
      bw.write("inline bool IsAlphanum(unsigned int c) {\n");
      bw.write("  return (kAlphanumTable[c/64] >> (c%64)) & 0x1;\n");
      bw.write("}\n");
      bw.write("\n");
    }
  }

  public static void genSource(String path) throws Exception {
    List<String> hexes = genHexData();
    try (BufferedWriter bw = new BufferedWriter(new FileWriter(path + ".cc"))) {
      bw.write(String.format("#include \"%s.h\"\n", path));
      bw.write("\n");
      bw.write("__attribute__((aligned(64))) const uint64_t kAlphanumTable[] = {\n");
      for (int i = 0; i < hexes.size(); i += 4) {
        bw.write("  ");
        bw.write(hexes.get(i));
        bw.write(", ");
        bw.write(hexes.get(i+1));
        bw.write(", ");
        bw.write(hexes.get(i+2));
        bw.write(", ");
        bw.write(hexes.get(i+3));
        bw.write(",\n");
      }
      bw.write("};\n");
      bw.write("\n");
    }
  }

  public static List<String> genHexData() {
    List<String> hexes = new ArrayList<>();
    for (int w = 0; w < (2<<16)/64; w++) {
      boolean[] word = new boolean[64];
      for (int i = 0; i < 64; i++) {
        int c = w * 64 + i;
        word[63-i] = Character.isLetterOrDigit((char)c);
      }
      String s = "0x";
      for (int i = 0; i < 16; i++) {
        int octet = 0;
        for (int j = 0; j < 4; j++) {
          octet <<= 1;
          if (word[i * 4 + j]) {
            octet |= 0x1;
          }
        }
        if (octet <= 9) {
          s += (char)('0' + octet);
        } else {
          s += (char)('a' + (octet-10));
        }
      }
      hexes.add(s);
    }
    return hexes;
  }
  
  public static void genImage(String prefix) throws Exception {
    //BufferedImage img = new BufferedImage(256, 256, BufferedImage.TYPE_BYTE_GRAY);
    BufferedImage img = new BufferedImage(256, 256, BufferedImage.TYPE_3BYTE_BGR);
    for (int i = 0; i < (1<<16); i++) {
      //int v = Character.isLetterOrDigit((char)i) ? 0xffffff : 0x0;
      int v = Character.isLetterOrDigit((char)i) ? 0xff0000 : 0x000000;
      img.setRGB(i%256, i/256, v);
    }
    ImageIO.write(img, "png", new File(prefix + ".png"));
  }

  public static void main(String[] args) throws Exception {
    genHeader("isalphanum");
    genSource("isalphanum");
    genImage("isalphanum");
  }

}

