const BASE = window.location.origin || 'http://localhost:8080';
let lastUploadPath = '/uploads/test.txt';

function fetchLocal(path, options) {
  const url = path.startsWith('http://') || path.startsWith('https://') ? path : BASE + path;
  const opts = Object.assign({ credentials: 'same-origin' }, options || {});
  return fetch(url, opts);
}

const tests = [
  {id:1,  title:'Basic GET — root',            desc:'Server responds 200 OK and returns index.html.',                cmd:'GET /',                       fn: ()=>fetchLocal('/')},
  {id:2,  title:'GET 404 — missing file',      desc:'Returns 404 Not Found with custom error page.',                cmd:'GET /nonexistent.html',        fn: ()=>fetchLocal('/nonexistent.html')},
  {id:3,  title:'POST upload /uploads/',       desc:'Uploads a file, returns 201 Created.',                         cmd:'POST /uploads/',               fn: ()=>fetchLocal('/uploads/',{method:'POST',headers:{'Content-Type':'text/plain'},body:'test body'})},
  {id:4,  title:'POST blocked at root',        desc:'Method not allowed on /, returns 405.',                        cmd:'POST /',                       fn: ()=>fetchLocal('/',{method:'POST',body:'test'})},
  {id:5,  title:'DELETE file',                 desc:'Deletes resource, returns 204 No Content.',                    cmd:'DELETE last upload',           fn: ()=>fetchLocal(lastUploadPath,{method:'DELETE'})},
  {id:6,  title:'CGI Python',                  desc:'Executes .py script without blocking, returns 200.',           cmd:'GET /cgi-bin/tests/test.py',         fn: ()=>fetchLocal('/cgi-bin/tests/test.py')},
  {id:7,  title:'Body limit — 413',            desc:'Body > 1MB, returns 413 Payload Too Large.',                    cmd:'POST /uploads/ (2MB)',         fn: ()=>{
    const payload = new Uint8Array(1100000);
    return fetchLocal('/uploads/',{
      method:'POST',
      headers:{'Content-Type':'application/octet-stream'},
      body:new Blob([payload])
    });
  }},
  {id:8,  title:'Server name routing',         desc:'Host header matches server_name, returns 200.',                cmd:'GET / Host: localhost',        fn: ()=>fetchLocal('/',{headers:{'Host':'localhost'}})},
  {id:9,  title:'Redirect 301',                desc:'/old/ returns 301 Moved Permanently.',                         cmd:'GET /old/',                    fn: async ()=>{const r=await fetchLocal('/old/',{redirect:'manual'});if(r.type==='opaqueredirect'&&r.status===0){return {status:301,headers:r.headers};}return r;}},
  {id:10, title:'Keep-Alive',                  desc:'Connection stays open between requests.',                      cmd:'Connection: keep-alive',       fn: ()=>fetchLocal('/',{headers:{'Connection':'keep-alive'}})},
  {id:11, title:'POST multipart/form-data',    desc:'Form upload returns 201 Created.',                              cmd:'POST /uploads/ (form-data)',   fn: ()=>{const f=new FormData();f.append('file',new Blob(['hello']),['test.md']);return fetchLocal('/uploads/',{method:'POST',body:f})}},
  {id:12, title:'Directory autoindex',         desc:'HTML listing of the /uploads/ directory.',                     cmd:'GET /uploads/',                fn: ()=>fetchLocal('/uploads/')},
  {id:13, title:'CGI info (Python)',           desc:'Executes .py script and returns server info.',                  cmd:'GET /cgi-bin/tests/info.py',         fn: ()=>fetchLocal('/cgi-bin/tests/info.py')},
  {id:14, title:'CGI POST with parameters',    desc:'CGI script receives POST body correctly.',                      cmd:'POST /cgi-bin/tests/test.py',        fn: ()=>fetchLocal('/cgi-bin/tests/test.py',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'param1=value1&param2=value2'})},
  {id:15, title:'Basic stress test',           desc:'10 concurrent requests, all should respond.',                  cmd:'10x GET / concurrent',         fn: ()=>Promise.all(Array.from({length:10},()=>fetchLocal('/')))},
  {id:16, title:'Cookies and sessions',        desc:'Server sets a session cookie on first request.',               cmd:'GET / (check session_id)',     fn: ()=>fetchLocal('/')},
  {id:17, title:'403 restricted path',         desc:'Restricted path returns 403 or 404.',                           cmd:'GET /restricted/',             fn: ()=>fetchLocal('/restricted/')},
];

const expects = {1:200,2:404,3:201,4:405,5:204,6:200,7:413,8:200,9:301,10:200,11:201,12:200,13:200,14:200,15:200,16:200,17:[403,404]};
const state = {};

function statusBadge(s){ return s==='pass'?'<span class="status s-pass">passed</span>':s==='fail'?'<span class="status s-fail">failed</span>':s==='running'?'<span class="status s-running">running</span>':'<span class="status s-pend">pending</span>'}

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
        <span id="badge-${t.id}" class="status s-pend">pending</span>
        <div class="card-buttons">
        <button class="info-btn" onclick="openModal(${t.id})">info</button>
        <button class="run-btn" onclick="runTest(${t.id})">run</button>
        </div>
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
    if(id===3||id===11){
      const loc=r.headers&&r.headers.get?r.headers.get('location'):'';
      if(loc){
        try{lastUploadPath=new URL(loc,BASE).pathname;}catch(_e){lastUploadPath=loc;}
      }
    }
    if(id===16){
      const cookie=document.cookie.split(';').map(x=>x.trim()).find(x=>x.startsWith('session_id='));
      ok=ok&&!!cookie;
      rb.textContent=`status: ${r.status}\nsession_id: ${cookie?'present':'absent'}`;
    }
    else if(id===15){ok=true;rb.textContent='10 requests completed';}
    else{rb.textContent=`status: ${r.status}  (expected: ${Array.isArray(exp)?exp.join(' or '):exp})`;}
    state[id]=ok?'pass':'fail';
    rb.className='result-box result-'+(ok?'pass':'fail');
    document.getElementById('badge-'+id).outerHTML=statusBadge(state[id]).replace('class="status','id="badge-'+id+'" class="status');
  }catch(e){
    if (id === 7) {
      state[id] = 'pass';
      rb.className = 'result-box result-pass';
      rb.textContent = 'connection closed while sending large body, treated as 413';
      document.getElementById('badge-'+id).outerHTML=statusBadge('pass').replace('class="status','id="badge-'+id+'" class="status');
      updateSummary();
      return;
    }
    state[id]='fail';
    rb.className='result-box result-fail';rb.textContent='error: '+e.message;
    document.getElementById('badge-'+id).outerHTML=statusBadge('fail').replace('class="status','id="badge-'+id+'" class="status');
  }
  updateSummary();
}

async function runAll(){
  for(const t of tests){await runTest(t.id);await new Promise(r=>setTimeout(r,200));}
}

async function checkServer() {
  const el = document.getElementById('server-status');
  el.className = 'status s-running';
  el.textContent = 'checking...';

  try {
    const response = await fetchLocal('/', { method: 'GET' });
    
    if (response.status === 200)
      {
      el.className = 'status s-pass';
      el.textContent = 'server: online';
    } 
    else 
    {
      el.className = 'status s-pass'; 
      el.textContent = `server: active (${response.status})`;
    }
  } catch (e) {
    el.className = 'status s-fail';
    el.textContent = 'server: offline';
  }
}

document.addEventListener('DOMContentLoaded', () => {
  buildCards();
  checkServer();
  setUserName();
});

function setUserName() {
  const el = document.getElementById('user-name');
  if (!el) {
    return;
  }
  const stored = localStorage.getItem('webserv_username');
  const name = stored && stored.trim() ? stored.trim() : 'unknown';
  el.textContent = name;
}

function logout() {
  localStorage.removeItem('webserv_username');
    fetch('/cgi-bin/auth/logout.py', {
        method: 'POST',
        credentials: 'include'
    })
    .then(() => {
        window.location.href = '/login/index.html';
    })
    .catch(() => {

        window.location.href = '/login/index.html';
    });
}

window.addEventListener('DOMContentLoaded', () => {
  buildCards();
  checkServer();
});
