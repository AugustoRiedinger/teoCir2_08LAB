function marker(m)

     presicion = 3;      

       for n = 1:m         

            [x,y] = ginput(1);

             s1 = mat2str(x,presicion); s2 = mat2str(y,presicion);

             s  = ["(" s1 "," s2 ")"];

             gtext(s,"fontsize",16);

                n--;

        endfor
endfunction
