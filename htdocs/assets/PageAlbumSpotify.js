import{C as v}from"./ContentWithHero.js";import{C as x}from"./CoverArtwork.js";import{L as A,M as S}from"./ModalDialogTrackSpotify.js";import{M}from"./ModalDialogAlbumSpotify.js";import{S as z}from"./spotify-web-api.js";import{_ as B,e as D,r as n,o as p,c as f,d as l,w as c,s as h,a,t as u,F as L,f as T,g as E,h as F}from"./index.js";const _={load(e){const s=new z;return s.setAccessToken(h.state.spotify.webapi_token),s.getAlbum(e.params.id,{market:h.state.spotify.webapi_country})},set(e,s){e.album=s}},N={name:"PageAlbumSpotify",components:{ContentWithHero:v,CoverArtwork:x,ListItemTrackSpotify:A,ModalDialogAlbumSpotify:M,ModalDialogTrackSpotify:S},beforeRouteEnter(e,s,r){_.load(e).then(m=>{r(t=>_.set(t,m))})},beforeRouteUpdate(e,s,r){const m=this;_.load(e).then(t=>{_.set(m,t),r()})},data(){return{album:{artists:[{}],tracks:{}},selected_track:{},show_details_modal:!1,show_track_details_modal:!1}},computed:{artwork_url(){return this.album.images&&this.album.images.length>0?this.album.images[0].url:""}},methods:{open_artist(){this.$router.push({name:"music-spotify-artist",params:{id:this.album.artists[0].id}})},play(){this.show_details_modal=!1,D.player_play_uri(this.album.uri,!0)},open_track_dialog(e){this.selected_track=e,this.show_track_details_modal=!0}}},P=["textContent"],R={class:"subtitle is-6 has-text-link"},V=["textContent"],W={class:"buttons fd-is-centered-mobile mt-5"},j=["textContent"],H=["textContent"],I=["onClick"];function O(e,s,r,m,t,i){const d=n("mdicon"),k=n("cover-artwork"),b=n("list-item-track-spotify"),w=n("modal-dialog-album-spotify"),g=n("modal-dialog-track-spotify"),y=n("content-with-hero");return p(),f("div",null,[l(y,null,{"heading-left":c(()=>[a("h1",{class:"title is-5",textContent:u(t.album.name)},null,8,P),a("h2",R,[a("a",{class:"has-text-link",onClick:s[0]||(s[0]=(...o)=>i.open_artist&&i.open_artist(...o)),textContent:u(t.album.artists[0].name)},null,8,V)]),a("div",W,[a("a",{class:"button is-small is-dark is-rounded",onClick:s[1]||(s[1]=(...o)=>i.play&&i.play(...o))},[l(d,{class:"icon",name:"shuffle",size:"16"}),a("span",{textContent:u(e.$t("page.spotify.album.shuffle"))},null,8,j)]),a("a",{class:"button is-small is-light is-rounded",onClick:s[2]||(s[2]=o=>t.show_details_modal=!0)},[l(d,{class:"icon",name:"dots-horizontal",size:"16"})])])]),"heading-right":c(()=>[l(k,{artwork_url:i.artwork_url,artist:t.album.artist,album:t.album.name,class:"is-clickable fd-has-shadow fd-cover fd-cover-medium-image",onClick:s[3]||(s[3]=o=>t.show_details_modal=!0)},null,8,["artwork_url","artist","album"])]),content:c(()=>[a("p",{class:"heading is-7 has-text-centered-mobile mt-5",textContent:u(e.$t("page.spotify.album.track-count",{count:t.album.tracks.total}))},null,8,H),(p(!0),f(L,null,T(t.album.tracks.items,(o,C)=>(p(),E(b,{key:o.id,track:o,position:C,context_uri:t.album.uri},{actions:c(()=>[a("a",{onClick:F(U=>i.open_track_dialog(o),["prevent","stop"])},[l(d,{class:"icon has-text-dark",name:"dots-vertical",size:"16"})],8,I)]),_:2},1032,["track","position","context_uri"]))),128)),l(w,{show:t.show_details_modal,album:t.album,onClose:s[4]||(s[4]=o=>t.show_details_modal=!1)},null,8,["show","album"]),l(g,{show:t.show_track_details_modal,track:t.selected_track,album:t.album,onClose:s[5]||(s[5]=o=>t.show_track_details_modal=!1)},null,8,["show","track","album"])]),_:1})])}const Y=B(N,[["render",O]]);export{Y as default};
