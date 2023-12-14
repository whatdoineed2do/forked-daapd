import{C as h}from"./ContentWithHeading.js";import{G as l,a as b}from"./GroupByList.js";import{I as g}from"./IndexButtonList.js";import{L as x}from"./ListComposers.js";import{T as C}from"./TabsMusic.js";import{_ as w,r,o as y,c as B,d as a,w as i,e as L,a as p,t as m}from"./index.js";import"./ModalDialogComposer.js";const c={load(t){return L.library_composers("music")},set(t,e){t.composers=new l(e.data),t.composers.group(b("name_sort"))}},E={name:"PageComposers",components:{ContentWithHeading:h,IndexButtonList:g,ListComposers:x,TabsMusic:C},beforeRouteEnter(t,e,s){c.load(t).then(n=>{s(o=>c.set(o,n))})},beforeRouteUpdate(t,e,s){if(!this.composers.isEmpty()){s();return}const n=this;c.load(t).then(o=>{c.set(n,o),s()})},data(){return{composers:new l}}},N={class:"fd-page-with-tabs"},$=["textContent"],k=["textContent"];function v(t,e,s,n,o,G){const _=r("tabs-music"),d=r("index-button-list"),u=r("list-composers"),f=r("content-with-heading");return y(),B("div",N,[a(_),a(f,null,{options:i(()=>[a(d,{index:o.composers.indexList},null,8,["index"])]),"heading-left":i(()=>[p("p",{class:"title is-4",textContent:m(t.$t("page.composers.title"))},null,8,$),p("p",{class:"heading",textContent:m(t.$t("page.composers.count",{count:o.composers.total}))},null,8,k)]),content:i(()=>[a(u,{composers:o.composers},null,8,["composers"])]),_:1})])}const H=w(E,[["render",v]]);export{H as default};
