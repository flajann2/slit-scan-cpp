#!/usr/bin/fish
ffmpeg -y -f lavfi -i "color=c=gray:s=1280x720:r=24:d=10" \
  -f lavfi -i "color=c=red:s=20x720:r=24:d=10" \
  -f lavfi -i "color=c=lime:s=20x720:r=24:d=10" \
  -f lavfi -i "color=c=blue:s=20x720:r=24:d=10" \
  -f lavfi -i "color=c=cyan:s=1280x20:r=24:d=10" \
  -f lavfi -i "color=c=magenta:s=1280x20:r=24:d=10" \
  -f lavfi -i "color=c=yellow:s=1280x20:r=24:d=10" \
  -filter_complex "[0][1]overlay=x='mod(t*256,W)':y=0[a];[a][2]overlay=x='mod(t*256+120,W)':y=0[b];[b][3]overlay=x='mod(t*256+240,W)':y=0[c];[c][4]overlay=x=0:y='mod(t*144,H)'[d];[d][5]overlay=x=0:y='mod(t*144+120,H)'[e];[e][6]overlay=x=0:y='mod(t*144+240,H)',format=yuv420p" \
  -c:v libx264 bars_test_gray.mp4


ffmpeg -y  -f lavfi -i "color=c=white:s=1280x720:r=24:d=10" \
  -f lavfi -i "color=c=red:s=20x720:r=24:d=10" \
  -f lavfi -i "color=c=lime:s=20x720:r=24:d=10" \
  -f lavfi -i "color=c=blue:s=20x720:r=24:d=10" \
  -f lavfi -i "color=c=cyan:s=1280x20:r=24:d=10" \
  -f lavfi -i "color=c=magenta:s=1280x20:r=24:d=10" \
  -f lavfi -i "color=c=yellow:s=1280x20:r=24:d=10" \
  -filter_complex "[0][1]overlay=x='mod(t*256,W)':y=0[a];[a][2]overlay=x='mod(t*256+120,W)':y=0[b];[b][3]overlay=x='mod(t*256+240,W)':y=0[c];[c][4]overlay=x=0:y='mod(t*144,H)'[d];[d][5]overlay=x=0:y='mod(t*144+120,H)'[e];[e][6]overlay=x=0:y='mod(t*144+240,H)',format=yuv420p" \
  -c:v libx264 bars_test_white.mp4

ffmpeg -y -f lavfi -i "color=c=white:s=1280x720:r=24:d=10" \
  -vf "format=gbrp,geq=r='st(0,mod((X+Y)*0.70711-283*T+14140,1414));st(1,mod((W-X+Y)*0.70711-283*T+14140,1414));if(gte(ld(1),0)*lt(ld(1),20),0,if(gte(ld(1),120)*lt(ld(1),140),255,if(gte(ld(1),240)*lt(ld(1),260),255,if(gte(ld(0),0)*lt(ld(0),20),255,if(gte(ld(0),120)*lt(ld(0),140),0,if(gte(ld(0),240)*lt(ld(0),260),0,r(X,Y)))))))':g='st(0,mod((X+Y)*0.70711-283*T+14140,1414));st(1,mod((W-X+Y)*0.70711-283*T+14140,1414));if(gte(ld(1),0)*lt(ld(1),20),255,if(gte(ld(1),120)*lt(ld(1),140),255,if(gte(ld(1),240)*lt(ld(1),260),0,if(gte(ld(0),0)*lt(ld(0),20),0,if(gte(ld(0),120)*lt(ld(0),140),255,if(gte(ld(0),240)*lt(ld(0),260),0,g(X,Y)))))))':b='st(0,mod((X+Y)*0.70711-283*T+14140,1414));st(1,mod((W-X+Y)*0.70711-283*T+14140,1414));if(gte(ld(1),0)*lt(ld(1),20),255,if(gte(ld(1),120)*lt(ld(1),140),0,if(gte(ld(1),240)*lt(ld(1),260),255,if(gte(ld(0),0)*lt(ld(0),20),0,if(gte(ld(0),120)*lt(ld(0),140),0,if(gte(ld(0),240)*lt(ld(0),260),255,b(X,Y)))))))',format=yuv420p" \
  -c:v libx264 diag_test_white.mp4

ffmpeg -y -f lavfi -i "color=c=gray:s=1280x720:r=24:d=10" \
  -vf "format=gbrp,geq=r='st(0,mod((X+Y)*0.70711-283*T+14140,1414));st(1,mod((W-X+Y)*0.70711-283*T+14140,1414));if(gte(ld(1),0)*lt(ld(1),20),0,if(gte(ld(1),120)*lt(ld(1),140),255,if(gte(ld(1),240)*lt(ld(1),260),255,if(gte(ld(0),0)*lt(ld(0),20),255,if(gte(ld(0),120)*lt(ld(0),140),0,if(gte(ld(0),240)*lt(ld(0),260),0,r(X,Y)))))))':g='st(0,mod((X+Y)*0.70711-283*T+14140,1414));st(1,mod((W-X+Y)*0.70711-283*T+14140,1414));if(gte(ld(1),0)*lt(ld(1),20),255,if(gte(ld(1),120)*lt(ld(1),140),255,if(gte(ld(1),240)*lt(ld(1),260),0,if(gte(ld(0),0)*lt(ld(0),20),0,if(gte(ld(0),120)*lt(ld(0),140),255,if(gte(ld(0),240)*lt(ld(0),260),0,g(X,Y)))))))':b='st(0,mod((X+Y)*0.70711-283*T+14140,1414));st(1,mod((W-X+Y)*0.70711-283*T+14140,1414));if(gte(ld(1),0)*lt(ld(1),20),255,if(gte(ld(1),120)*lt(ld(1),140),0,if(gte(ld(1),240)*lt(ld(1),260),255,if(gte(ld(0),0)*lt(ld(0),20),0,if(gte(ld(0),120)*lt(ld(0),140),0,if(gte(ld(0),240)*lt(ld(0),260),255,b(X,Y)))))))',format=yuv420p" \
  -c:v libx264 diag_test_gray.mp4
