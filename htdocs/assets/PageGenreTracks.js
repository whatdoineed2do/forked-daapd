import{_ as x,G as C,e as p,r as a,o as w,c as v,d as r,w as c,a as s,t as d}from"./index.js";import{a as G,d as B,G as g}from"./GroupByList.js";import{C as R}from"./ContentWithHeading.js";import{C as E}from"./ControlDropdown.js";import{I as L}from"./IndexButtonList.js";import{L as T}from"./ListTracks.js";import{M as q}from"./ModalDialogGenre.js";import"./spotify-web-api.js";const m={load(e){return Promise.all([p.library_genre(e.params.name,e.query.media_kind),p.library_genre_tracks(e.params.name,e.query.media_kind)])},set(e,t){e.genre=t[0].data,e.tracks_list=new g(t[1].data.tracks)}},N={name:"PageGenreTracks",components:{ContentWithHeading:R,ControlDropdown:E,IndexButtonList:L,ListTracks:T,ModalDialogGenre:q},beforeRouteEnter(e,t,l){m.load(e).then(_=>{l(n=>m.set(n,_))})},beforeRouteUpdate(e,t,l){if(!this.tracks_list.isEmpty()){l();return}const _=this;m.load(e).then(n=>{m.set(_,n),l()})},data(){return{genre:{},groupby_options:[{id:1,name:this.$t("page.genre.sort-by.name"),options:G("title_sort")},{id:2,name:this.$t("page.genre.sort-by.rating"),options:B("rating",{direction:"desc"})}],media_kind:this.$route.query.media_kind,show_details_modal:!1,tracks_list:new g}},computed:{expression(){return`genre is "${this.genre.name}" and media_kind is ${this.media_kind}`},selected_groupby_option_id:{get(){return this.$store.state.genre_tracks_sort},set(e){this.$store.commit(C,e)}},tracks(){const e=this.groupby_options.find(t=>t.id===this.selected_groupby_option_id);return this.tracks_list.group(e.options),this.tracks_list}},methods:{open_genre(){this.show_details_modal=!1,this.$router.push({name:"genre-albums",params:{name:this.genre.name},query:{media_kind:this.media_kind}})},play(){p.player_play_expression(this.expression,!0)}}},z={class:"columns"},D={class:"column"},P=["textContent"],S=["textContent"],I={class:"buttons is-centered"},M=["textContent"],O={class:"heading has-text-centered-mobile"},U=["textContent"],V=s("span",null," | ",-1),j=["textContent"];function A(e,t,l,_,n,o){const h=a("index-button-list"),k=a("control-dropdown"),u=a("mdicon"),b=a("list-tracks"),y=a("modal-dialog-genre"),f=a("content-with-heading");return w(),v("div",null,[r(f,null,{options:c(()=>[r(h,{index:o.tracks.indexList},null,8,["index"]),s("div",z,[s("div",D,[s("p",{class:"heading mb-5",textContent:d(e.$t("page.genre.sort-by.title"))},null,8,P),r(k,{value:o.selected_groupby_option_id,"onUpdate:value":t[0]||(t[0]=i=>o.selected_groupby_option_id=i),options:n.groupby_options},null,8,["value","options"])])])]),"heading-left":c(()=>[s("p",{class:"title is-4",textContent:d(n.genre.name)},null,8,S)]),"heading-right":c(()=>[s("div",I,[s("a",{class:"button is-small is-light is-rounded",onClick:t[1]||(t[1]=i=>n.show_details_modal=!0)},[r(u,{class:"icon",name:"dots-horizontal",size:"16"})]),s("a",{class:"button is-small is-dark is-rounded",onClick:t[2]||(t[2]=(...i)=>o.play&&o.play(...i))},[r(u,{class:"icon",name:"shuffle",size:"16"}),s("span",{textContent:d(e.$t("page.genre.shuffle"))},null,8,M)])])]),content:c(()=>[s("p",O,[s("a",{class:"has-text-link",onClick:t[3]||(t[3]=(...i)=>o.open_genre&&o.open_genre(...i)),textContent:d(e.$t("page.genre.album-count",{count:n.genre.album_count}))},null,8,U),V,s("span",{textContent:d(e.$t("page.genre.track-count",{count:n.genre.track_count}))},null,8,j)]),r(b,{tracks:o.tracks,expression:o.expression},null,8,["tracks","expression"]),r(y,{show:n.show_details_modal,genre:n.genre,media_kind:n.media_kind,onClose:t[4]||(t[4]=i=>n.show_details_modal=!1)},null,8,["show","genre","media_kind"])]),_:1})])}const Z=x(N,[["render",A]]);export{Z as default};
