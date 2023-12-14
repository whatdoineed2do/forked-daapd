import{_,e as r,r as m,o as c,c as u,d as l,w as p,a as t,t as i,k as C,T as h}from"./index.js";const f={name:"ModalDialogArtistSpotify",props:["show","artist"],emits:["close"],methods:{play(){this.$emit("close"),r.player_play_uri(this.artist.uri,!1)},queue_add(){this.$emit("close"),r.queue_add(this.artist.uri)},queue_add_next(){this.$emit("close"),r.queue_add_next(this.artist.uri)},open_artist(){this.$emit("close"),this.$router.push({name:"music-spotify-artist",params:{id:this.artist.id}})}}},x={key:0,class:"modal is-active"},y={class:"modal-content fd-modal-card"},k={class:"card"},g={class:"card-content"},v={class:"title is-4"},q=["textContent"],w={class:"content is-small"},z=["textContent"],$=["textContent"],b=["textContent"],B=["textContent"],D={class:"card-footer"},M=["textContent"],N=["textContent"],S=["textContent"];function V(o,s,n,j,A,a){const d=m("mdicon");return c(),u("div",null,[l(h,{name:"fade"},{default:p(()=>[n.show?(c(),u("div",x,[t("div",{class:"modal-background",onClick:s[0]||(s[0]=e=>o.$emit("close"))}),t("div",y,[t("div",k,[t("div",g,[t("p",v,[t("a",{class:"has-text-link",onClick:s[1]||(s[1]=(...e)=>a.open_artist&&a.open_artist(...e)),textContent:i(n.artist.name)},null,8,q)]),t("div",w,[t("p",null,[t("span",{class:"heading",textContent:i(o.$t("dialog.spotify.artist.popularity"))},null,8,z),t("span",{class:"title is-6",textContent:i([n.artist.popularity,n.artist.followers.total].join(" / "))},null,8,$)]),t("p",null,[t("span",{class:"heading",textContent:i(o.$t("dialog.spotify.artist.genres"))},null,8,b),t("span",{class:"title is-6",textContent:i(n.artist.genres.join(", "))},null,8,B)])])]),t("footer",D,[t("a",{class:"card-footer-item has-text-dark",onClick:s[2]||(s[2]=(...e)=>a.queue_add&&a.queue_add(...e))},[l(d,{class:"icon",name:"playlist-plus",size:"16"}),t("span",{class:"is-size-7",textContent:i(o.$t("dialog.spotify.artist.add"))},null,8,M)]),t("a",{class:"card-footer-item has-text-dark",onClick:s[3]||(s[3]=(...e)=>a.queue_add_next&&a.queue_add_next(...e))},[l(d,{class:"icon",name:"playlist-play",size:"16"}),t("span",{class:"is-size-7",textContent:i(o.$t("dialog.spotify.artist.add-next"))},null,8,N)]),t("a",{class:"card-footer-item has-text-dark",onClick:s[4]||(s[4]=(...e)=>a.play&&a.play(...e))},[l(d,{class:"icon",name:"play",size:"16"}),t("span",{class:"is-size-7",textContent:i(o.$t("dialog.spotify.artist.play"))},null,8,S)])])])]),t("button",{class:"modal-close is-large","aria-label":"close",onClick:s[5]||(s[5]=e=>o.$emit("close"))})])):C("",!0)]),_:1})])}const E=_(f,[["render",V]]);export{E as M};
