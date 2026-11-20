// Run: node Tests/PreviewTests.cjs (no npm dependencies).
const fs=require('node:fs'),path=require('node:path'),vm=require('node:vm'),assert=require('node:assert/strict');
const html=fs.readFileSync(path.join(__dirname,'../Anteprima.html'),'utf8');
const character=fs.readFileSync(path.join(__dirname,'../Assets/Character.js'),'utf8');
const script=html.split('<script>')[1].split('</script>')[0];
const nodes={};
const classList=()=>({values:new Set(),toggle(n,force){const on=force??!this.values.has(n);on?this.values.add(n):this.values.delete(n);return on;}});
const makeNode=(id)=>({value:id==='bpm'?'120':id==='size'?'100':'',selectedIndex:1,textContent:'',innerHTML:'',dataset:{},attrs:{'aria-pressed':'true'},classList:classList(),style:{setProperty(){}},getAttribute(n){return this.attrs[n]},setAttribute(n,v){this.attrs[n]=v},querySelector(){return this.child??={textContent:''}}});
const rates=[.5,1,2].map(rate=>({...makeNode('rate'),dataset:{rate:String(rate)}}));
const beats=[0,1,2,3].map(()=>makeNode('beat'));
const body={classList:classList()};
const context={assert,console,performance:{now:()=>0},requestAnimationFrame:()=>{},document:{body,getElementById:id=>nodes[id]??=makeNode(id),querySelectorAll:q=>q==='[data-rate]'?rates:beats}};
vm.runInNewContext(character+'\n'+script+`
for(let n=0;n<12;n++)for(let b=-8;b<=8;b+=.0625){let p=pose(b,n,true);for(const v of Object.values(p))assert(Number.isFinite(v));const art=render(p);assert(!art.includes('NaN'));assert(art.includes('linearGradient'));}
$('play').onclick();assert.equal(playing,false);$('play').onclick();assert.equal(playing,true);
for(let n=0;n<12;n++){$('preset').selectedIndex=n;tick(last+16);assert($('cat').innerHTML.includes('<path'));}
$('preset').selectedIndex=0;$('prev').onclick();assert.equal($('preset').selectedIndex,11);$('next').onclick();assert.equal($('preset').selectedIndex,0);
$('meter').value='3/4';$('meter').onchange();
assert.equal(meterNumerator,3);assert.equal($('signature').textContent,'3/4');assert.equal($('beats').innerHTML,'<i>1</i><i>2</i><i>3</i>');
$('meter').value='6/8';$('meter').onchange();
assert.equal(meterDenominator,8);assert($('beats').innerHTML.endsWith('<i>6</i>'));
$('bpm').value='bad';tick(last+16);assert(!Number.isNaN(beat));
$('size').value='160';$('size').oninput();assert.equal($('sizeOut').textContent,'160 %');$('size').ondblclick();assert.equal($('sizeOut').textContent,'100 %');
$('overlay').onclick();assert.equal($('overlay').getAttribute('aria-pressed'),'false');
$('clean').onclick();
`,context);
assert(body.classList.values.has('clean'));
rates[0].onclick();assert(rates[0].classList.values.has('active'));
console.log('Preview OK: 12 shaded dances, preset navigation, pause, BPM, rate, size/reset and overlay controls.');
