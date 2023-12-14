import{_ as x,C as y,e as _,r as i,o as w,c as v,d as a,w as m,a as e,t as l}from"./index.js";import{C as B}from"./ContentWithHeading.js";import{C as R}from"./ControlDropdown.js";import{a as L,d as T,G as h}from"./GroupByList.js";import{I as O}from"./IndexButtonList.js";import{L as P}from"./ListTracks.js";import{M as S}from"./ModalDialogComposer.js";import"./spotify-web-api.js";const d={load(o){return Promise.all([_.library_composer(o.params.name),_.library_composer_tracks(o.params.name)])},set(o,t){o.composer=t[0].data,o.tracks_list=new h(t[1].data.tracks)}},z={name:"PageComposerTracks",components:{ContentWithHeading:B,ControlDropdown:R,IndexButtonList:O,ListTracks:P,ModalDialogComposer:S},beforeRouteEnter(o,t,c){d.load(o).then(p=>{c(s=>d.set(s,p))})},beforeRouteUpdate(o,t,c){const p=this;d.load(o).then(s=>{d.set(p,s),c()})},data(){return{composer:{},groupby_options:[{id:1,name:this.$t("page.composer.sort-by.name"),options:L("title_sort")},{id:2,name:this.$t("page.composer.sort-by.rating"),options:T("rating",{direction:"desc"})}],show_details_modal:!1,tracks_list:new h}},computed:{expression(){return`composer is "${this.composer.name}" and media_kind is music`},selected_groupby_option_id:{get(){return this.$store.state.composer_tracks_sort},set(o){this.$store.commit(y,o)}},tracks(){const o=this.groupby_options.find(t=>t.id===this.selected_groupby_option_id);return this.tracks_list.group(o.options),this.tracks_list}},methods:{open_albums(){this.show_details_modal=!1,this.$router.push({name:"music-composer-albums",params:{name:this.composer.name}})},play(){_.player_play_expression(this.expression,!0)}}},D={class:"columns"},E={class:"column"},M=["textContent"],N=["textContent"],G={class:"buttons is-centered"},I=["textContent"],U={class:"heading has-text-centered-mobile"},V=["textContent"],j=e("span",null," | ",-1),A=["textContent"];function H(o,t,c,p,s,n){const g=i("index-button-list"),b=i("control-dropdown"),u=i("mdicon"),k=i("list-tracks"),C=i("modal-dialog-composer"),f=i("content-with-heading");return w(),v("div",null,[a(f,null,{options:m(()=>[a(g,{index:n.tracks.indexList},null,8,["index"]),e("div",D,[e("div",E,[e("p",{class:"heading mb-5",textContent:l(o.$t("page.artist.sort-by.title"))},null,8,M),a(b,{value:n.selected_groupby_option_id,"onUpdate:value":t[0]||(t[0]=r=>n.selected_groupby_option_id=r),options:s.groupby_options},null,8,["value","options"])])])]),"heading-left":m(()=>[e("p",{class:"title is-4",textContent:l(s.composer.name)},null,8,N)]),"heading-right":m(()=>[e("div",G,[e("a",{class:"button is-small is-light is-rounded",onClick:t[1]||(t[1]=r=>s.show_details_modal=!0)},[a(u,{class:"icon",name:"dots-horizontal",size:"16"})]),e("a",{class:"button is-small is-dark is-rounded",onClick:t[2]||(t[2]=(...r)=>n.play&&n.play(...r))},[a(u,{class:"icon",name:"shuffle",size:"16"}),e("span",{textContent:l(o.$t("page.composer.shuffle"))},null,8,I)])])]),content:m(()=>[e("p",U,[e("a",{class:"has-text-link",onClick:t[3]||(t[3]=(...r)=>n.open_albums&&n.open_albums(...r)),textContent:l(o.$t("page.composer.album-count",{count:s.composer.album_count}))},null,8,V),j,e("span",{textContent:l(o.$t("page.composer.track-count",{count:s.composer.track_count}))},null,8,A)]),a(k,{tracks:n.tracks,expression:n.expression},null,8,["tracks","expression"]),a(C,{show:s.show_details_modal,composer:s.composer,onClose:t[4]||(t[4]=r=>s.show_details_modal=!1)},null,8,["show","composer"])]),_:1})])}const Z=x(z,[["render",H]]);export{Z as default};
