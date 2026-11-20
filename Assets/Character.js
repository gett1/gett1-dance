// Browser preview companion for the articulated native JUCE drawing in Source/Cat.h.
const pi=Math.PI;
function pose(beat,n,active){let p={x:0,y:0,lean:0,head:0,leftArm:0,rightArm:0,leftFoot:0,rightFoot:0,turn:1,squash:1,tail:0,leftLeg:0,rightLeg:0,leftSlide:0,rightSlide:0,leftToe:0,rightToe:0};if(!active||!Number.isFinite(beat))return p;const a=2*pi*(beat%8),s=Math.sin(a),c=Math.cos(a),slow=Math.sin(a/2);p.tail=.15*slow;switch(n){
        case 0: p.head=0.14*s; p.y=2*c; p.leftArm=.05*s; p.rightArm=-.05*s; break;
        case 1: p.y=-10*(1-c)/2; p.squash=1+.06*c; p.lean=.08*slow;
                p.leftArm=-.3-.22*s; p.rightArm=.3+.22*s; p.head=.1*s; break;
        case 2: p.x=14*slow; p.lean=.12*slow; p.y=-4*Math.abs(s);
                p.leftFoot=8*Math.max(0.0,slow); p.rightFoot=8*Math.max(0.0,-slow);
                p.leftArm=.35*slow; p.rightArm=.35*slow; break;
        case 3: p.y=-14*(1-c)/2; p.x=6*slow; p.lean=.08*slow;
                p.leftArm=1.0+.35*s; p.rightArm=-1.0-.35*s;
                p.leftFoot=10*Math.max(0.0,s); p.rightFoot=10*Math.max(0.0,-s); break;
        case 4: p.lean=.12*slow; p.head=-.18*slow;
                p.leftArm=.8+.5*Math.sin(a/2); p.rightArm=-.8+.5*Math.sin(a/2-pi/2);
                p.y=-3*(1-c); break;
        case 5: p.turn=Math.cos(a/4); p.y=-8*(1-c)/2;
                p.leftArm=.6; p.rightArm=-.6; p.head=.07*s; break;
        case 6: // Running Man: alternating raised knees and sliding support foot.
                p.y=-5*(1-c)/2;p.lean=.05*slow;
                p.leftFoot=20*Math.max(0.0,slow);p.rightFoot=20*Math.max(0.0,-slow);
                p.leftSlide=-15*Math.cos(a/2);p.rightSlide=15*Math.cos(a/2);
                p.leftLeg=.45*slow;p.rightLeg=-.45*slow;
                p.leftArm=.45*slow;p.rightArm=.45*slow;p.head=.06*s;break;
        case 7: // Shuffle / T-step: heel-toe pivots and alternating half-beat lifts.
                p.x=10*slow;p.y=-5*Math.abs(s);p.lean=.07*slow;
                p.leftFoot=14*Math.max(0.0,s);p.rightFoot=14*Math.max(0.0,-s);
                p.leftSlide=10*c;p.rightSlide=-10*c;
                p.leftToe=.5*slow;p.rightToe=.5*slow;
                p.leftLeg=.25*s;p.rightLeg=-.25*s;
                p.leftArm=.25+.3*slow;p.rightArm=-.25+.3*slow;break;
        case 8: // Griddy-inspired heel taps and low arm pumps.
                p.y=-6*(1-c)/2;p.lean=.1*slow;p.head=.08*s;
                p.leftFoot=18*Math.max(0.0,slow);p.rightFoot=18*Math.max(0.0,-slow);
                p.leftSlide=10*Math.sin(a/2+pi/3);p.rightSlide=-p.leftSlide;
                p.leftToe=-.25*slow;p.rightToe=.25*slow;
                p.leftArm=-.45+.35*slow;p.rightArm=.45+.35*slow;break;
        case 9: // Floss-inspired counter-swing of arms and hips.
                p.x=-8*slow;p.lean=-.12*slow;p.y=-3*(1-c)/2;
                p.leftArm=-.4+.9*slow;p.rightArm=.4+.9*slow;
                p.leftFoot=8*Math.max(0.0,slow);p.rightFoot=8*Math.max(0.0,-slow);
                p.leftLeg=.25*slow;p.rightLeg=.25*slow;p.head=.05*slow;break;
        case 10: // Charleston-style alternating kicks and toe swivels.
                p.y=-4*(1-c)/2;p.lean=.08*slow;
                p.leftSlide=16*slow;p.rightSlide=-16*slow;
                p.leftLeg=.6*slow;p.rightLeg=-.6*slow;
                p.leftFoot=12*Math.max(0.0,slow);p.rightFoot=12*Math.max(0.0,-slow);
                p.leftToe=.45*s;p.rightToe=-.45*s;
                p.leftArm=.35*slow;p.rightArm=.35*slow;break;
        case 11: // Heel Toe: paired ankle pivots with opposing foot travel.
                p.x=5*slow;p.lean=.08*slow;p.head=-.08*slow;
                p.leftSlide=-11*slow;p.rightSlide=11*slow;
                p.leftToe=.55*slow;p.rightToe=.55*slow;
                p.leftFoot=8*(1+c)/2;p.rightFoot=8*(1-c)/2;
                p.leftLeg=.28*slow;p.rightLeg=.28*slow;
                p.leftArm=.3+.18*slow;p.rightArm=-.3+.18*slow;break;
        default: break;
}return p;}
function render(p){
 let id=0,defs='';
 const grad=(x,y,x2,y2,back=false)=>{const name='mat'+id++;defs+=`<linearGradient id="${name}" gradientUnits="userSpaceOnUse" x1="${x}" y1="${y}" x2="${x2}" y2="${y2}"><stop stop-color="${back?'#f35db8':'#ff9bdc'}"/><stop offset=".42" stop-color="${back?'#d73b94':'#f35db8'}"/><stop offset="1" stop-color="${back?'#a62672':'#bd297f'}"/></linearGradient>`;return `url(#${name})`;};
 const oval=(x,y,w,h,back=false)=>`<ellipse cx="${x+w/2}" cy="${y+h/2}" rx="${w/2}" ry="${h/2}" fill="${grad(x,y,x+w,y+h,back)}" stroke="#8d225f" stroke-width="1.35"/>`;
 const capsule=(a,b,w,back=false)=>{const x=Math.min(a.x,b.x)-w/2,y=Math.min(a.y,b.y)-w/2,x2=Math.max(a.x,b.x)+w/2,y2=Math.max(a.y,b.y)+w/2,fill=grad(x,y,x2,y2,back);const d=`M${a.x} ${a.y}L${b.x} ${b.y}`;return `<path d="${d}" fill="none" stroke="#8d225f" stroke-width="${w+1.35}" stroke-linecap="round"/><path d="${d}" fill="none" stroke="${fill}" stroke-width="${w-1.35}" stroke-linecap="round"/>`;};
 let art='';
 const tail=`M35 84C91 108 107 ${78+p.tail*65} 101 ${54+p.tail*65}`;
 art+=`<path d="${tail}" fill="none" stroke="#8d225f" stroke-width="26.35" stroke-linecap="round"/><path d="${tail}" fill="none" stroke="${grad(23,42,119,108,true)}" stroke-width="23.65" stroke-linecap="round"/>`;

 defs+='<radialGradient id="belly" gradientUnits="userSpaceOnUse" cx="-12" cy="61" r="56"><stop stop-color="#ffb2e2" stop-opacity=".22"/><stop offset="1" stop-color="#f35db8" stop-opacity="0"/></radialGradient>';
 art+=oval(-43,17,86,110)+'<ellipse cx="-1" cy="74.5" rx="24" ry="34.5" fill="url(#belly)"/><ellipse cx="0" cy="28.5" rx="33" ry="8.5" fill="#8d225f" opacity=".4"/>';
 for(const side of [-1,1]){
  const lift=side<0?p.leftFoot:p.rightFoot,swing=side<0?p.leftLeg:p.rightLeg,slide=side<0?p.leftSlide:p.rightSlide,toe=side<0?p.leftToe:p.rightToe;
  const hip={x:side*22,y:108},knee={x:side*25+12*Math.sin(swing)+slide*.4,y:126-lift*.55},ankle={x:side*29+22*Math.sin(swing)+slide,y:139-lift};
  art+=capsule(hip,knee,25,true)+capsule(knee,ankle,23,true)+`<g transform="rotate(${toe*180/pi} ${ankle.x} ${ankle.y})">`+oval(ankle.x-17,ankle.y-11,34,25)+`<path d="M${ankle.x-9} ${ankle.y+7}h17" stroke="#8d225f" stroke-opacity=".45" stroke-width=".8"/></g>`;
 }
 const head='M-83-68L-61-124Q-58-132-54-125L-25-85H25L54-125Q58-132 61-124L83-68C115-15 78 28 0 28C-78 28-115-15-83-68Z';
 art+=`<g transform="rotate(${p.head*180/pi})"><path d="${head}" fill="${grad(-95,-130,95,28)}" stroke="#8d225f" stroke-width="1.35"/>`;
 for(const side of [-1,1]){const name='ear'+side;defs+=`<linearGradient id="${name}" x1="0" y1="0" x2="0" y2="1"><stop stop-color="#bb327e" stop-opacity=".65"/><stop offset="1" stop-color="#f35db8" stop-opacity=".1"/></linearGradient>`;art+=`<path d="M${side*58}-111L${side*70}-70L${side*36}-78Z" fill="url(#${name})"/>`;}
 defs+=`<clipPath id="headclip"><path d="${head}"/></clipPath><radialGradient id="shine" gradientUnits="userSpaceOnUse" cx="-40" cy="-52" r="105"><stop stop-color="white" stop-opacity=".12"/><stop offset="1" stop-color="white" stop-opacity="0"/></radialGradient>`;
 art+=`<path d="${head}" fill="url(#shine)"/>`;
 const alpha=Math.max(0,Math.min(1,(p.turn-.08)*2.2));
 for(const x of [-30,30]){const d=`M${x-14}-26C${x-9}-10 ${x+9}-10 ${x+14}-26`;art+=`<g opacity="${alpha}"><path d="${d}" fill="none" stroke="#ffc2e7" stroke-opacity=".7" stroke-width="8.5" stroke-linecap="round" transform="translate(0 1.4)"/><path d="${d}" fill="none" stroke="#30202d" stroke-width="7.5" stroke-linecap="round"/></g>`;}
 art+='</g>';
 const rot=(x,y,a)=>({x:x*Math.cos(a)-y*Math.sin(a),y:x*Math.sin(a)+y*Math.cos(a)});
 for(const side of [-1,1]){const a=side<0?p.leftArm:p.rightArm,shoulder={x:side*43,y:42},r=rot(side*23,12,a),elbow={x:shoulder.x+r.x,y:shoulder.y+r.y},r2=rot(side*19,-3,a*1.35),paw={x:elbow.x+r2.x,y:elbow.y+r2.y};art+=`<ellipse cx="${shoulder.x+.5}" cy="${shoulder.y+3}" rx="12.5" ry="13" fill="#8d225f" opacity=".5"/>`+capsule(shoulder,elbow,22)+capsule(elbow,paw,21)+oval(paw.x-12,paw.y-12,24,25)+`<ellipse cx="${paw.x-2}" cy="${paw.y-4.5}" rx="4" ry="2.5" fill="white" opacity=".2"/>`;}
 const depth=Math.sign(p.turn||1)*(.22+.78*Math.abs(p.turn));
 return `<defs>${defs}</defs><g transform="translate(${p.x} ${p.y}) rotate(${p.lean*180/pi} 0 90) scale(${depth*(2-p.squash)} ${p.squash})">${art}</g>`;
}
