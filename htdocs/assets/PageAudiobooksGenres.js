import{G as _,a as b}from"./GroupByList.js";import{C as f}from"./ContentWithHeading.js";import{I as h}from"./IndexButtonList.js";import{L as x}from"./ListGenres.js";import{T as k}from"./TabsAudiobooks.js";import{_ as w,r as a,o as C,c as B,d as r,w as d,e as G,a as c,t as l}from"./index.js";import"./ModalDialogGenre.js";const i={load(e){return G.library_genres("audiobook")},set(e,o){e.genres=o.data,e.genres=new _(o.data),e.genres.group(b("name_sort"))}},L={name:"PageAudiobooksGenres",components:{ContentWithHeading:f,IndexButtonList:h,ListGenres:x,TabsAudiobooks:k},beforeRouteEnter(e,o,n){i.load(e).then(s=>{n(t=>i.set(t,s))})},beforeRouteUpdate(e,o,n){const s=this;i.load(e).then(t=>{i.set(s,t),n()})},data(){return{genres:new _}}},y={class:"fd-page-with-tabs"},A=["textContent"],N=["textContent"];function $(e,o,n,s,t,E){const p=a("tabs-audiobooks"),u=a("index-button-list"),g=a("list-genres"),m=a("content-with-heading");return C(),B("div",y,[r(p),r(m,null,{options:d(()=>[r(u,{index:t.genres.indexList},null,8,["index"])]),"heading-left":d(()=>[c("p",{class:"title is-4",textContent:l(e.$t("page.genres.title"))},null,8,A),c("p",{class:"heading",textContent:l(e.$t("page.genres.count",{count:t.genres.total}))},null,8,N)]),content:d(()=>[r(g,{genres:t.genres,media_kind:"audiobook"},null,8,["genres"])]),_:1})])}const D=w(L,[["render",$]]);export{D as default};
