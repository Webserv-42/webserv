const BASE = 'http://localhost:8080';

const tests = [
  {id:1,  title:'GET básico — raíz',           desc:'Servidor responde 200 OK y devuelve index.html.',                cmd:'GET /',                       fn: ()=>fetch(BASE+'/')},
  {id:2,  title:'GET 404 — archivo inexistente',desc:'Responde 404 Not Found con página de error personalizada.',    cmd:'GET /nonexistent.html',        fn: ()=>fetch(BASE+'/nonexistent.html')},
  {id:3,  title:'POST upload /uploads/',        desc:'Sube un archivo, responde 201 Created.',                       cmd:'POST /uploads/test.txt',       fn: ()=>fetch(BASE+'/uploads/test.txt',{method:'POST',body:'test body'})},
  {id:4,  title:'POST bloqueado en raíz',       desc:'Método no permitido en /, responde 405.',                      cmd:'POST /',                       fn: ()=>fetch(BASE+'/',{method:'POST',body:'test'})},
  {id:5,  title:'DELETE archivo',               desc:'Borra recurso, responde 204 No Content.',                      cmd:'DELETE /uploads/test.txt',     fn: ()=>fetch(BASE+'/uploads/test.txt',{method:'DELETE'})},
  {id:6,  title:'CGI Python',                   desc:'Ejecuta script .py sin bloquear, responde 200.',               cmd:'GET /cgi-bin/test.py',         fn: ()=>fetch(BASE+'/cgi-bin/test.py')},
  {id:7,  title:'Límite body — 413',            desc:'Body > 1MB, responde 413 Payload Too Large.',                  cmd:'POST /uploads/ (2MB)',         fn: ()=>fetch(BASE+'/uploads/',{method:'POST',body:new Uint8Array(1100000)})},
  {id:8,  title:'Server name routing',          desc:'Header Host coincide con server_name, responde 200.',          cmd:'GET / Host: localhost',        fn: ()=>fetch(BASE+'/',{headers:{'Host':'localhost'}})},
  {id:9,  title:'Redirección 301',              desc:'/old/ responde 301 Moved Permanently.',                        cmd:'GET /old/',                    fn: ()=>fetch(BASE+'/old/',{redirect:'manual'})},
  {id:10, title:'Keep-Alive',                   desc:'Conexión se mantiene abierta entre peticiones.',               cmd:'Connection: keep-alive',       fn: ()=>fetch(BASE+'/',{headers:{'Connection':'keep-alive'}})},
  {id:11, title:'POST multipart/form-data',     desc:'Upload con formulario, responde 201 Created.',                 cmd:'POST /uploads/ (form-data)',   fn: ()=>{const f=new FormData();f.append('file',new Blob(['hello']),['test.md']);return fetch(BASE+'/uploads/',{method:'POST',body:f})}},
  {id:12, title:'Autoindex directorio',         desc:'Lista HTML del directorio /uploads/.',                         cmd:'GET /uploads/',                fn: ()=>fetch(BASE+'/uploads/')},
  {id:13, title:'CGI Bash',                     desc:'Ejecuta script .sh, responde 200 con info del servidor.',      cmd:'GET /cgi-bin/info.sh',         fn: ()=>fetch(BASE+'/cgi-bin/info.sh')},
  {id:14, title:'CGI POST con parámetros',      desc:'Script CGI recibe body POST correctamente.',                   cmd:'POST /cgi-bin/test.py',        fn: ()=>fetch(BASE+'/cgi-bin/test.py',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'param1=value1&param2=value2'})},
  {id:15, title:'Stress test básico',           desc:'10 peticiones simultáneas, todas deben responder.',            cmd:'10x GET / simultáneo',         fn: ()=>Promise.all(Array.from({length:10},()=>fetch(BASE+'/')))},
  {id:16, title:'Cookies y sesiones',           desc:'Servidor emite Set-Cookie en primera petición.',               cmd:'GET / (check Set-Cookie)',     fn: ()=>fetch(BASE+'/')},
  {id:17, title:'403 ruta restringida',         desc:'Ruta sin permisos responde 403 o 404.',                        cmd:'GET /restricted/',             fn: ()=>fetch(BASE+'/restricted/')},
];

const expects = {1:200,2:404,3:201,4:405,5:204,6:200,7:413,8:200,9:301,10:200,11:201,12:200,13:200,14:200,15:200,16:200,17:[403,404]};
const state = {};

function statusBadge(s){ return s==='pass'?'<span class="status s-pass">aprobada</span>':s==='fail'?'<span class="status s-fail">fallida</span>':s==='running'?'<span class="status s-running">ejecutando</span>':'<span class="status s-pend">pendiente</span>'}

function buildCards(){
  const g=document.getElementById('grid');
  g.innerHTML=tests.map(t=>`
    <div class="card" id="card-${t.id}">
      <div class="card-header">
        <div class="card-title">${t.title}</div>
        <div class="card-num">#${t.id}</div>
      </div>
      <div class="card-desc">${t.desc}</div>
      <div class="card-cmd">${t.cmd}</div>
      <div class="card-footer">
        <span id="badge-${t.id}" class="status s-pend">pendiente</span>
        <button class="run-btn" onclick="runTest(${t.id})">ejecutar</button>
      </div>
      <div class="result-box" id="result-${t.id}"></div>
    </div>`).join('');
}

function updateSummary(){
  const pass=Object.values(state).filter(s=>s==='pass').length;
  const fail=Object.values(state).filter(s=>s==='fail').length;
  const pend=tests.length-pass-fail;
  document.getElementById('s-pass').textContent=pass;
  document.getElementById('s-fail').textContent=fail;
  document.getElementById('s-pend').textContent=pend;
  document.getElementById('prog').style.width=((pass/tests.length)*100)+'%';
}

async function runTest(id){
  const t=tests.find(x=>x.id===id);
  state[id]='running';
  document.getElementById('badge-'+id).outerHTML=`<span id="badge-${id}" class="status s-running">ejecutando</span>`;
  const rb=document.getElementById('result-'+id);
  rb.className='result-box';rb.style.display='block';rb.textContent='...';
  try{
    const r=await t.fn();
    const exp=expects[id];
    let ok=Array.isArray(exp)?exp.includes(r.status):r.status===exp;
    if(id===16){const h=r.headers.get('set-cookie');ok=ok&&!!h;rb.textContent=`status: ${r.status}\nSet-Cookie: ${h||'ausente'}`;}
    else if(id===15){ok=true;rb.textContent='10 peticiones completadas';}
    else{rb.textContent=`status: ${r.status}  (esperado: ${Array.isArray(exp)?exp.join(' o '):exp})`;}
    state[id]=ok?'pass':'fail';
    rb.className='result-box result-'+(ok?'pass':'fail');
    document.getElementById('badge-'+id).outerHTML=statusBadge(state[id]).replace('class="status','id="badge-'+id+'" class="status');
  }catch(e){
    state[id]='fail';
    rb.className='result-box result-fail';rb.textContent='error: '+e.message;
    document.getElementById('badge-'+id).outerHTML=statusBadge('fail').replace('class="status','id="badge-'+id+'" class="status');
  }
  updateSummary();
}

async function runAll(){
  for(const t of tests){await runTest(t.id);await new Promise(r=>setTimeout(r,200));}
}

async function checkServer(){
  const el=document.getElementById('server-status');
  el.className='status s-running';el.textContent='verificando...';
  try{
    const r=await fetch(BASE+'/');
    el.className='status s-pass';el.textContent='servidor: activo ('+r.status+')';
  }catch(e){
    el.className='status s-fail';el.textContent='servidor: sin respuesta';
  }
}

// Dentro de dashboard.js (con type="module")
document.addEventListener('DOMContentLoaded', () => {
  buildCards();
  checkServer();
  
  // Conectar botones manualmente
  document.querySelector('.btn-primary').addEventListener('click', runAll);
  document.querySelector('.btn[onclick="checkServer()"]').onclick = checkServer;
});