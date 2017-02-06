#!/usr/bin/ruby

require 'asciidoctor'
require 'asciidoctor/extensions'
require 'asciidoctor/cli'
require 'tmpdir'
require 'cgi'
require 'digest'

module MathRenderer
  FONT_SIZE_PX = 16      # main html font size
  PX_IN_EX = 8.5
  DPI_PER_PX   = 14.454  # selected so that height of simple single-line formula matches height of main html font
  SCALING_FACTOR = 4.0   # dvipng produces preciser results with higher DPI; must be at least 2.0 because of HI-DPI monitors

  def gentex formula
    tex = ''
    tex += "\\documentclass[10pt]{article}\n"
    tex += "\\usepackage[utf8]{inputenc}\n"
    tex += "\\usepackage{amsmath}\n"
    tex += "\\usepackage{amssymb}\n"
    # used by dvipng to extract baseline (--depth) from *.dvi
    tex += "\\usepackage[active,textmath]{preview}\n"
    tex += "\\usepackage{xcolor}\n"

    # Fonts: https://en.wikibooks.org/wiki/LaTeX/Fonts
    #tex += "\\renewcommand{\\sfdefault}{phv}\n"
    #tex += "\\renewcommand{\\familydefault}{\\sfdefault}\n"
    #tex += "\\SetSymbolFont{letters}{normal}{OML}{zplm}{m}{it}\n"
    tex += "\\SetSymbolFont{operators}{normal}{OT1}{ptm}{m}{n}\n"

    tex += "\\newcommand{\\less}{<}\n"
    tex += "\\newcommand{\\gtr}{>}\n"

    tex += "\\pagestyle{empty}\n"
    tex += "\\begin{document}\n"
    tex += %($\\color{black!85}\\displaystyle{#{formula}}$\n)
    tex += "\\end{document}\n"

    return tex 
  end

  def convert1 formula, dir
    texfile = File.join(dir, 'formula.tex')
    dvifile = File.join(dir, 'formula.dvi')
    pngfile = File.join(dir, 'formula.png')
    svgfile = File.join(dir, 'formula.svg')

    # tex -> dvi
    texdata = gentex(formula)
    puts texdata
    IO.binwrite(texfile, texdata)
    if !system(%(cd #{dir} && latex -halt-on-error #{texfile}))
      raise 'latex execution failed'
    end

    # dvi -> svg
    # without --cache=none dvisvgm may produce different XML on each invocation
    # which is not convenient for storing result in VCS
    # (difference is in order of tags -- not in perceptual content)
    if !system(%(cd #{dir} && dvisvgm --cache=none -n -b min -e #{dvifile}))
      raise 'dvisvgm execution failed'
    end
    svgdata = IO.binread(svgfile) 

    # dvi -> png (to get dimensions and baseline)
    dpi = (1.0*FONT_SIZE_PX*DPI_PER_PX*SCALING_FACTOR).round
    output=`cd #{dir} && dvipng -T tight -D #{dpi} --depth -o #{pngfile} #{dvifile}`
    if !$?.success?
      raise 'dvipng execution failed'
    end
    if !output.match(/depth=(\d+)/)
      raise 'dvipng execution failed'
    end
    pngdepth = $~[1].to_i
    pngwidth, pngheight = IO.read(pngfile)[0x10..0x18].unpack('NN')

    return {
      'svgdata' => svgdata,
      'exheight' => 1.0*pngheight/FONT_SIZE_PX/SCALING_FACTOR,
      'exvalign' => -1.0*pngdepth/FONT_SIZE_PX/SCALING_FACTOR,
      'pxheight' => 1.0*pngheight/FONT_SIZE_PX/SCALING_FACTOR*PX_IN_EX,
      'formula' => formula
    }
  end

  def convert formula
    Dir.mktmpdir do |dir|
      return convert1(formula, dir)
    end
  end

  def store img
    dir = ENV['MATH_OUTPUT']
    if !dir
      raise 'No MATH_OUTPUT env var set'
    end
    name = %(math-#{Digest::SHA256.hexdigest(img['formula'])[0,16]}.svg)
    path = File.join(dir, name)
    IO.binwrite(File.join(dir, name), img['svgdata'])
    return name
  end
end

class MathInlineMacroProcessor < Asciidoctor::Extensions::InlineMacroProcessor
  use_dsl
  named :math
  using_format :short

  include MathRenderer

  def process parent, target, attrs
    formula = target
    if !formula or formula == ''
      return ''
    end

    img = convert(formula)
    path = store(img)
    html = ''
    html += %(<img)
    html +=   %( src="#{CGI.escapeHTML(path)}")
    html +=   %( class="inlinemath")
    html +=   %( style=")
    html +=     %(height:#{CGI.escapeHTML(img['exheight'].round(3).to_s())}ex;)
    html +=     %(vertical-align:#{CGI.escapeHTML(img['exvalign'].round(3).to_s())}ex;)
    html +=   %(")
    html +=   %( alt="#{CGI.escapeHTML(img['formula'])}")
    html += %(/>)
    return html
  end
end

class MathBlockProcessor < Asciidoctor::Extensions::BlockProcessor
  use_dsl
  named :math
  on_context :listing

  include MathRenderer

  def process parent, reader, attrs
    formula = reader.lines.join("\n")
    if !formula or formula == ''
      return nil
    end

    img = convert(formula)
    path = store(img)
    attrs['target'] = path
    attrs['alt'] = formula
    if !attrs['width'] and !attrs['height']
      attrs['height'] = img['pxheight'].round()
    end
    if !attrs['align'] and !attrs['role'] and !attrs['float']
      attrs['role'] = 'text-indent'
    end
    return create_image_block parent, attrs
  end
end


class DisqusBlockMacroProcessor < Asciidoctor::Extensions::BlockMacroProcessor
  use_dsl
  named :disqus

  def process parent, target, attrs
    site_id = 'andreigudkov'
    page_id = target
    page_url = 'http://gudok.xyz/' + CGI.escape(page_id) + '/'

    # todo: escape javascript
    html = %(
      <div id='disqus_thread'></div>
      <script>
        var disqus_config = function () {
          this.page.url = '#{page_url}';
          this.page.identifier = '#{page_id}';
        };

        (function() {
          var d = document, s = d.createElement('script');
          s.src = '//#{site_id}.disqus.com/embed.js';
          s.setAttribute('data-timestamp', +new Date());
          (d.head || d.body).appendChild(s);
        })();
      </script>
      <noscript>Please enable JavaScript to view the <a href='https://disqus.com/?ref_noscript'>comments powered by Disqus.</a></noscript>
    )

    create_pass_block parent, html, attrs, subs: nil
  end
end


Asciidoctor::Extensions.register do
  inline_macro MathInlineMacroProcessor
  block MathBlockProcessor
  block_macro DisqusBlockMacroProcessor
end
invoker = Asciidoctor::Cli::Invoker.new ARGV
GC.start
invoker.invoke!
exit invoker.code

