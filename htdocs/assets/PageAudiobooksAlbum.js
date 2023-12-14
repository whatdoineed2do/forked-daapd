import{C}from"./ContentWithHero.js";import{C as g}from"./CoverArtwork.js";import{G as p}from"./GroupByList.js";import{L as x}from"./ListTracks.js";import{M as v}from"./ModalDialogAlbum.js";import{_ as y,e as _,r as n,o as A,c as B,d as l,w as c,a as e,t as u}from"./index.js";import"./spotify-web-api.js";const d={load(s){return Promise.all([_.library_album(s.params.id),_.library_album_tracks(s.params.id)])},set(s,o){s.album=o[0].data,s.tracks=new p(o[1].data)}},z={name:"PageAudiobooksAlbum",components:{ContentWithHero:C,CoverArtwork:g,ListTracks:x,ModalDialogAlbum:v},beforeRouteEnter(s,o,r){d.load(s).then(i=>{r(t=>d.set(t,i))})},beforeRouteUpdate(s,o,r){const i=this;d.load(s).then(t=>{d.set(i,t),r()})},data(){return{album:{},show_details_modal:!1,tracks:new p}},methods:{open_artist(){this.show_details_modal=!1,this.$router.push({name:"audiobooks-artist",params:{id:this.album.artist_id}})},play(){_.player_play_uri(this.album.uri,!1)}}},L=["textContent"],P={class:"subtitle is-6 has-text-link"},D=["textContent"],E={class:"buttons fd-is-centered-mobile mt-5"},G=["textContent"],M=["textContent"];function N(s,o,r,i,t,m){const b=n("mdicon"),k=n("cover-artwork"),h=n("list-tracks"),f=n("modal-dialog-album"),w=n("content-with-hero");return A(),B("div",null,[l(w,null,{"heading-left":c(()=>[e("h1",{class:"title is-5",textContent:u(t.album.name)},null,8,L),e("h2",P,[e("a",{class:"has-text-link",onClick:o[0]||(o[0]=(...a)=>m.open_artist&&m.open_artist(...a)),textContent:u(t.album.artist)},null,8,D)]),e("div",E,[e("a",{class:"button is-small is-dark is-rounded",onClick:o[1]||(o[1]=(...a)=>m.play&&m.play(...a))},[l(b,{class:"icon",name:"play",size:"16"}),e("span",{textContent:u(s.$t("page.audiobooks.album.play"))},null,8,G)]),e("a",{class:"button is-small is-light is-rounded",onClick:o[2]||(o[2]=a=>t.show_details_modal=!0)},[l(b,{class:"icon",name:"dots-horizontal",size:"16"})])])]),"heading-right":c(()=>[l(k,{artwork_url:t.album.artwork_url,artist:t.album.artist,album:t.album.name,class:"is-clickable fd-has-shadow fd-cover fd-cover-medium-image",onClick:o[3]||(o[3]=a=>t.show_details_modal=!0)},null,8,["artwork_url","artist","album"])]),content:c(()=>[e("p",{class:"heading is-7 has-text-centered-mobile mt-5",textContent:u(s.$t("page.audiobooks.album.track-count",{count:t.album.track_count}))},null,8,M),l(h,{tracks:t.tracks,uris:t.album.uri},null,8,["tracks","uris"]),l(f,{show:t.show_details_modal,album:t.album,media_kind:"audiobook",onClose:o[4]||(o[4]=a=>t.show_details_modal=!1)},null,8,["show","album"])]),_:1})])}const U=y(z,[["render",N]]);export{U as default};
